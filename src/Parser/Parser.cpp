#include "Parser/Parser.h"

#include <assert.h>

#include <cctype>
#include <fstream>

#include "Parser/ArrayField.h"
#include "Parser/SingleField.h"
#include "Util.h"

namespace Parser {

// ----------------------------------------------------------------------------
// Parser::Input_ definition and implementation.
// ----------------------------------------------------------------------------

//! Class wrapping a stack of istream instances. This is used to implement
//! constant value substitution.
class Parser::Input_ {
  public:
    void Push(std::istream *input) {
        inputs_.push(input);
    }
    void Pop(std::istream *input) {
        assert(! inputs_.empty());
        assert(inputs_.top() == input);
        inputs_.pop();
    }
    std::istream & Get(char &c) {
        return Top_().get(c);
    }
    char Peek() const {
        return Top_().peek();
    }
    bool IsAtEOF() const {
        return inputs_.empty() || Top_().eof();
    }
    void PutBack(char c) {
        Top_().putback(c);
    }
    bool ParseInt(int &i) {
        return ! ! (Top_() >> i);
    }
    bool ParseFloat(float &f) {
        bool ret = ! ! (Top_() >> f);
        return ret;
    }

  private:
    std::stack<std::istream *> inputs_;
    std::istream & Top_() const {
        assert(! inputs_.empty());
        return *inputs_.top();
    }
};

// ----------------------------------------------------------------------------
// Parser implementation.
// ----------------------------------------------------------------------------

Parser::Parser(const std::vector<ObjectSpec> &object_specs) :
    object_specs_(object_specs),
    input_ptr_(new Input_),
    input_(*input_ptr_) {
    BuildSpecMaps_();
}

Parser::~Parser() {
}

ObjectPtr Parser::ParseFile(const std::filesystem::path &path) {
    if (path.is_relative())
        path_ = (base_path_ / path).native();
    else
        path_ = path.native();

    std::ifstream in(path_);
    if (in.fail())
        Throw_("Failed to open file");

    return ParseStream(in);
}

ObjectPtr Parser::ParseStream(std::istream &in) {
    if (path_.empty())
        path_ = "<input stream>";
    cur_line_ = 1;
    input_.Push(&in);
    ObjectPtr obj = ParseObject_();
    input_.Pop(&in);
    return obj;
}

void Parser::BuildSpecMaps_() {
    assert(object_spec_map_.empty());
    assert(field_spec_map_.empty());

    for (const ObjectSpec &obj_spec: object_specs_) {
        // Check for duplicate types.
        if (object_spec_map_.find(obj_spec.type_name) !=
            object_spec_map_.end()) {
            throw Exception("Multiple object specs for type '" +
                            obj_spec.type_name);
        }
        object_spec_map_[obj_spec.type_name] = &obj_spec;

        // Add FieldSpecs, checking for duplicate names and bad counts.
        for (const FieldSpec &field_spec: obj_spec.field_specs) {
            const std::string qual_name =
                GetQualifiedFieldName_(obj_spec.type_name, field_spec.name);
            if (field_spec.count == 0) {
                throw Exception("FieldSpec for '" + qual_name +
                                "' has a zero count");
            }
            if ((field_spec.type == ValueType::kObject ||
                 field_spec.type == ValueType::kObjectList) &&
                field_spec.count > 1) {
                throw Exception("FieldSpec for '" + qual_name +
                                "' of type " + Util::EnumName(field_spec.type) +
                                " has a count > 1");
            }
            if (field_spec_map_.find(qual_name) != field_spec_map_.end()) {
                throw Exception("Multiple field specs for field '" +
                                field_spec.name + " in object " +
                                obj_spec.type_name);
            }
            field_spec_map_[qual_name] = &field_spec;
        }
    }
}

ObjectPtr Parser::ParseIncludedFile_() {
    ParseChar_('<');
    std::string path;
    char c;
    while (input_.Get(c) && c != '>')
        path += c;
    if (input_.IsAtEOF())
        Throw_("EOF reached before closing '>'");
    if (path.empty())
        Throw_("Invalid empty path for included file");

    if (! object_stack_.empty())
        object_stack_.top()->included_paths.push_back(path);

    return ParseFile(path);
}

ObjectPtr Parser::ParseObject_() {
    // Check for an included file: "<...path...>"
    if (PeekChar_() == '<')
        return ParseIncludedFile_();

    // Read and validate the object type name.
    std::string name = ParseName_();
    const ObjectSpec &spec = GetObjectSpec_(name);

    // If the next character is a quotation mark, parse the name.
    std::string obj_name;
    if (PeekChar_() == '"') {
        obj_name = ParseQuotedString_();

        // If the next character is a ';', this is a reference to an existing
        // object.
        if (PeekChar_() == ';') {
            ParseChar_(';');
            return GetObjectByName_(obj_name, spec);
        }
    }

    // Construct a new Object.
    ObjectPtr obj(new Object(spec, path_, cur_line_));
    object_stack_.push(obj);
    ParseChar_('{');
    if (PeekChar_() != '}')  // Valid to have an object with no fields.
        obj->fields = ParseFields_(spec);
    ParseChar_('}');
    assert(object_stack_.top() == obj);
    object_stack_.pop();

    // If there was a name given, store it in the map.
    if (! obj_name.empty()) {
        obj->name = obj_name;
        const std::string qual_name =
            GetQualifiedObjectName_(spec.type_name, obj_name);
        object_name_map_[qual_name] = obj;
    }

    return obj;
}

std::vector<ObjectPtr> Parser::ParseObjectList_() {
    std::vector<ObjectPtr> objects;
    ParseChar_('[');
    while (true) {
        // If the next character is a closing brace, stop. An empty list of
        // objects is valid.
        if (PeekChar_() == ']')
            break;

        objects.push_back(ParseObject_());

        // Parse the trailing comma.
        char c = PeekChar_();
        if (c == ',')
            ParseChar_(',');
    }
    ParseChar_(']');
    return objects;
}

std::vector<FieldPtr> Parser::ParseFields_(const ObjectSpec &obj_spec) {
    std::vector<FieldPtr> fields;
    while (true) {
        std::string name = ParseName_();
        ParseChar_(':');

        // Get the expected type for the field.
        const FieldSpec &spec =
            GetFieldSpec_(GetQualifiedFieldName_(obj_spec.type_name, name));

        // Parse the value based on the type.
        if (spec.count == 1)
            fields.push_back(ParseSingleFieldValue_(spec));
        else
            fields.push_back(ParseArrayFieldValue_(spec));

        // Parse the trailing comma.
        char c = PeekChar_();
        if (c == ',') {
            ParseChar_(',');
            c = PeekChar_();
        }
        // If there was no comma, there must be a closing brace.
        else if (c != '}')
            Throw_(std::string("Expected ',' or '}', got '") + c + "'");

        // If the next character is a closing brace, stop.
        if (c == '}')
            break;
    }
    return fields;
}

FieldPtr Parser::ParseSingleFieldValue_(const FieldSpec &spec) {
    SkipWhiteSpace_();
    return FieldPtr(new SingleField_(spec, ParseValue_(spec)));
}

FieldPtr Parser::ParseArrayFieldValue_(const FieldSpec &spec) {
    std::vector<Value> values;
    values.reserve(spec.count);
    for (uint32_t i = 0; i < spec.count; ++i)
        values.push_back(ParseValue_(spec));
    return FieldPtr(new ArrayField_(spec, values));
}

Value Parser::ParseValue_(const FieldSpec &spec) {
    SkipWhiteSpace_();

    Value value;
    switch (spec.type) {
      case ValueType::kBool:
        value = ParseBool_();
        break;
      case ValueType::kInteger:
        value = ParseInteger_();
        break;
      case ValueType::kFloat:
        value = ParseFloat_();
        break;
      case ValueType::kString:
        value = ParseQuotedString_();
        break;
      case ValueType::kObject:
        value = ParseObject_();
        break;
      case ValueType::kObjectList:
        value = ParseObjectList_();
        break;
      default:                            // LCOV_EXCL_LINE
        Throw_("Unexpected field type");  // LCOV_EXCL_LINE
    }
    return value;
}

std::string Parser::ParseName_() {
    SkipWhiteSpace_();
    std::string s = "";
    char c;
    while (input_.Get(c)) {
        if (isalnum(c) || c == '_') {
            s += c;
        }
        else {
            input_.PutBack(c);
            break;
        }
    }
    if (s.empty())
        Throw_("Invalid empty type name");
    if (! isalpha(s[0]) && s[0] != '_')
        Throw_("Invalid type name '" + s + "'");
    return s;
}

bool Parser::ParseBool_() {
    bool val;
    std::string s;
    char c;
    while (input_.Get(c) && isalpha(c))
        s += c;
    if (! isalpha(c))
        input_.PutBack(c);
    if (Util::StringsEqualNoCase(s, "t") ||
        Util::StringsEqualNoCase(s, "true")) {
        val = true;
    }
    else if (Util::StringsEqualNoCase(s, "f") ||
        Util::StringsEqualNoCase(s, "false")) {
        val = false;
    }
    else {
        Throw_("Invalid bool value '" + s + "'");
    }
    return val;
}

int Parser::ParseInteger_() {
    int i;
    if (! input_.ParseInt(i))
        Throw_("Invalid integer value");
    return i;
}

float Parser::ParseFloat_() {
    float f;
    if (! input_.ParseFloat(f))
        Throw_("Invalid float value");
    return f;
}

std::string Parser::ParseQuotedString_() {
    // For simplicity, this assumes that there are no escaped characters or
    // newlines in the string.
    std::string s;
    ParseChar_('"');
    char c;
    while (input_.Get(c) && c != '"')
        s += c;
    return s;
}

void Parser::ParseChar_(char expected_c) {
    SkipWhiteSpace_();
    char c;
    if (input_.Get(c) && c == expected_c)
        return;
    if (input_.IsAtEOF())
        Throw_(std::string("Expected '") + expected_c + "', got EOF");
    else
        Throw_(std::string("Expected '") + expected_c + "', got '" + c + "'");
}

char Parser::PeekChar_() {
    SkipWhiteSpace_();
    return (char) input_.Peek();
}

void Parser::SkipWhiteSpace_() {
    char c;
    while (input_.Get(c)) {
        // Check for comments; read to end of line.
        if (c == '#') {
            while (input_.Get(c) && c != '\n')
                ;
        }
        if (c == '\n')
            ++cur_line_;

        else if (! isspace(c)) {
            input_.PutBack(c);
            return;
        }
    }
}

const ObjectSpec & Parser::GetObjectSpec_(const std::string &name) {
    auto it = object_spec_map_.find(name);
    if (it == object_spec_map_.end())
        Throw_(std::string("Unknown object type '") + name + "'");
    return *it->second;
}

const FieldSpec & Parser::GetFieldSpec_(const std::string &name) {
    auto it = field_spec_map_.find(name);
    if (it == field_spec_map_.end())
        Throw_(std::string("Unknown field name '") + name + "'");
    return *it->second;
}

const ObjectPtr & Parser::GetObjectByName_(const std::string &name,
                                           const ObjectSpec &spec) {
    const std::string qual_name = GetQualifiedObjectName_(spec.type_name, name);
    auto it = object_name_map_.find(qual_name);
    if (it == object_name_map_.end())
        Throw_(std::string("Invalid reference to object of type '") +
               spec.type_name + "' with name '" + name + "'");
    return it->second;
}

void Parser::Throw_(const std::string &msg) {
    throw Exception(path_, cur_line_, msg);
}

}  // namespace Parser
