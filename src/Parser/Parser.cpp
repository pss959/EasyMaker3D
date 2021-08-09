#include "Parser/Parser.h"

#include <cctype>
#include <fstream>

#include "Parser/ArrayField.h"
#include "Parser/SingleField.h"
#include "Util.h"

namespace Parser {

Parser::Parser(const std::vector<ObjectSpec> &object_specs) :
    object_specs_(object_specs) {
    BuildSpecMaps_();
}

Parser::~Parser() {
}

ObjectPtr Parser::ParseFile(const std::string &path) {
    path_ = path;

    std::ifstream in(path);
    if (in.fail())
        Throw_("Failed to open file");

    return ParseStream(in);
}

ObjectPtr Parser::ParseStream(std::istream &in) {
    if (path_.empty())
        path_ = "<input stream>";
    cur_line_ = 1;
    return ParseObject_(in);
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

ObjectPtr Parser::ParseObject_(std::istream &in) {
    // Read and validate the object type name.
    std::string name = ParseName_(in);
    const ObjectSpec &spec = GetObjectSpec_(name);

    // If the next character is a quotation mark, parse the name.
    std::string obj_name;
    if (PeekChar_(in) == '"') {
        obj_name = ParseQuotedString_(in);

        // If the next character is a ';', this is a reference to an existing
        // object.
        if (PeekChar_(in) == ';') {
            ParseChar_(in, ';');
            return GetObjectByName_(obj_name, spec);
        }
    }

    // Construct a new Object.
    ObjectPtr obj(new Object(spec, path_, cur_line_));
    ParseChar_(in, '{');
    if (PeekChar_(in) != '}')  // Valid to have an object with no fields.
        obj->fields = ParseFields_(in, spec);
    ParseChar_(in, '}');

    // If there was a name given, store it in the map.
    if (! obj_name.empty()) {
        const std::string qual_name =
            GetQualifiedObjectName_(spec.type_name, obj_name);
        object_name_map_[qual_name] = obj;
    }

    return obj;
}

std::vector<ObjectPtr> Parser::ParseObjectList_(std::istream &in) {
    std::vector<ObjectPtr> objects;
    ParseChar_(in, '[');
    while (true) {
        // If the next character is a closing brace, stop. An empty list of
        // objects is valid.
        if (PeekChar_(in) == ']')
            break;

        objects.push_back(ParseObject_(in));

        // Parse the trailing comma.
        char c = PeekChar_(in);
        if (c == ',')
            ParseChar_(in, ',');
    }
    ParseChar_(in, ']');
    return objects;
}

std::vector<FieldPtr> Parser::ParseFields_(std::istream &in,
                                           const ObjectSpec &obj_spec) {
    std::vector<FieldPtr> fields;
    while (true) {
        std::string name = ParseName_(in);
        ParseChar_(in, ':');

        // Get the expected type for the field.
        const FieldSpec &spec =
            GetFieldSpec_(GetQualifiedFieldName_(obj_spec.type_name, name));

        // Parse the value based on the type.
        if (spec.count == 1)
            fields.push_back(ParseSingleFieldValue_(in, spec));
        else
            fields.push_back(ParseArrayFieldValue_(in, spec));

        // Parse the trailing comma.
        char c = PeekChar_(in);
        if (c == ',') {
            ParseChar_(in, ',');
            c = PeekChar_(in);
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

FieldPtr Parser::ParseSingleFieldValue_(std::istream &in,
                                        const FieldSpec &spec) {
    SkipWhiteSpace_(in);
    return FieldPtr(new SingleField_(spec, ParseValue_(in, spec)));
}

FieldPtr Parser::ParseArrayFieldValue_(std::istream &in,
                                       const FieldSpec &spec) {
    std::vector<Value> values;
    values.reserve(spec.count);
    for (uint32_t i = 0; i < spec.count; ++i)
        values.push_back(ParseValue_(in, spec));
    return FieldPtr(new ArrayField_(spec, values));
}

Value Parser::ParseValue_(std::istream &in, const FieldSpec &spec) {
    SkipWhiteSpace_(in);

    Value value;
    switch (spec.type) {
      case ValueType::kBool:
        value = ParseBool_(in);
        break;
      case ValueType::kInteger:
        value = ParseInteger_(in);
        break;
      case ValueType::kFloat:
        value = ParseFloat_(in);
        break;
      case ValueType::kString:
        value = ParseQuotedString_(in);
        break;
      case ValueType::kObject:
        value = ParseObject_(in);
        break;
      case ValueType::kObjectList:
        value = ParseObjectList_(in);
        break;
      default:                            // LCOV_EXCL_LINE
        Throw_("Unexpected field type");  // LCOV_EXCL_LINE
    }
    return value;
}

std::string Parser::ParseName_(std::istream &in) {
    SkipWhiteSpace_(in);
    std::string s = "";
    char c;
    while (in.get(c)) {
        if (isalnum(c) || c == '_') {
            s += c;
        }
        else {
            in.putback(c);
            break;
        }
    }
    if (s.empty())
        Throw_("Invalid empty type name");
    if (! isalpha(s[0]) && s[0] != '_')
        Throw_("Invalid type name '" + s + "'");
    return s;
}

bool Parser::ParseBool_(std::istream &in) {
    bool val;
    std::string s;
    char c;
    while (in.get(c) && isalpha(c))
        s += c;
    if (! isalpha(c))
        in.putback(c);
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

int Parser::ParseInteger_(std::istream &in) {
    int i;
    if (! (in >> i))
        Throw_("Invalid integer value");
    return i;
}

float Parser::ParseFloat_(std::istream &in) {
    float f;
    if (! (in >> f))
        Throw_("Invalid float value");
    return f;
}

std::string Parser::ParseQuotedString_(std::istream &in) {
    // For simplicity, this assumes that there are no escaped characters or
    // newlines in the string.
    std::string s;
    ParseChar_(in, '"');
    char c;
    while (in.get(c) && c != '"')
        s += c;
    return s;
}

void Parser::ParseChar_(std::istream &in, char expected_c) {
    SkipWhiteSpace_(in);
    char c;
    if (in.get(c) && c == expected_c)
        return;
    if (in.eof())
        Throw_(std::string("Expected '") + expected_c + "', got EOF");
    else
        Throw_(std::string("Expected '") + expected_c + "', got '" + c + "'");
}

char Parser::PeekChar_(std::istream &in) {
    SkipWhiteSpace_(in);
    return (char) in.peek();
}

void Parser::SkipWhiteSpace_(std::istream &in) {
    char c;
    while (in.get(c)) {
        // Check for comments; read to end of line.
        if (c == '#') {
            while (in.get(c) && c != '\n')
                ;
        }
        if (c == '\n')
            ++cur_line_;

        else if (! isspace(c)) {
            in.putback(c);
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
