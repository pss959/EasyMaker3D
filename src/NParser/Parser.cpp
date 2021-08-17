#include "NParser/Parser.h"

#include <assert.h>

#include <fstream>

#include "NParser/Scanner.h"
#include "Util/General.h"

namespace NParser {

Parser::Parser() : scanner_(new Scanner) {
}

Parser::~Parser() {
}

void Parser::RegisterObjectType(const std::string &type_name,
                                const std::vector<FieldSpec> &field_specs,
                                const CreationFunc &creation_func) {
    assert(! Util::MapContains(object_spec_map_, type_name));
    ObjectSpec_ spec;
    spec.field_specs   = field_specs;
    spec.creation_func = creation_func;
    object_spec_map_[type_name] = spec;
}

ObjectPtr Parser::ParseFile(const Util::FilePath &path) {
    std::ifstream in(path);
    if (in.fail())
        scanner_->Throw("Failed to open file");
    scanner_->PushInputStream(path, in);
    ObjectPtr obj = ParseObject_();
    scanner_->PopInputStream();
    return obj;
}

ObjectPtr Parser::ParseString(const std::string &str) {
    scanner_->PushStringInput(str);
    ObjectPtr obj = ParseObject_();
    scanner_->PopInputStream();
    return obj;
}

ObjectPtr Parser::ParseObject_() {
    // Check for an included file: "<...path...>"
    if (scanner_->PeekChar() == '<')
        return ParseIncludedFile_();

    std::string type_name = scanner_->ScanName();

    // If the next character is a quotation mark, parse the name.
    std::string obj_name;
    if (scanner_->PeekChar() == '"') {
        obj_name = scanner_->ScanQuotedString();

        // If the next character is a ';', this is a reference to an existing
        // object.
        if (scanner_->PeekChar() == ';') {
            scanner_->ScanExpectedChar(';');
            return FindObject_(type_name, obj_name);
        }
    }

    // Get the ObjectSpec_ for the type of object.
    const ObjectSpec_ &spec = GetObjectSpec_(type_name);

    // Invoke the creation function.
    ObjectPtr obj(spec.creation_func());
    obj->SetTypeName_(type_name);
    obj->SetName_(obj_name);
    scanner_->ScanExpectedChar('{');

    // Create an ObjectData_ instance for the object and add constants to it,
    // if there are any.
    ObjectData_ data{ obj };
    if (scanner_->PeekChar() == '[')
        ParseConstants_(*obj, data.constants_map);

    object_stack_.push_back(data);

    if (scanner_->PeekChar() != '}')  // Valid to have an object with no fields.
        ParseFields_(*obj, spec.field_specs);

    scanner_->ScanExpectedChar('}');
    assert(object_stack_.back().object == obj);
    object_stack_.pop_back();

    // If the object has a name, store it in the map.
    if (! obj_name.empty())
        object_name_map_[BuildObjectNameKey_(type_name, obj_name)] = obj;

    return obj;
}

std::vector<ObjectPtr> Parser::ParseObjectList_() {
    std::vector<ObjectPtr> objects;
    scanner_->ScanExpectedChar('[');
    while (true) {
        // If the next character is a closing brace, stop. An empty list of
        // objects is valid.
        if (scanner_->PeekChar() == ']')
            break;

        objects.push_back(ParseObject_());

        // Parse the trailing comma.
        char c = scanner_->PeekChar();
        if (c == ',')
            scanner_->ScanExpectedChar(',');
    }
    scanner_->ScanExpectedChar(']');
    return objects;
}

ObjectPtr Parser::ParseIncludedFile_() {
    scanner_->ScanExpectedChar('<');
    std::string path = scanner_->ScanQuotedString();
    scanner_->ScanExpectedChar('>');
    if (path.empty())
        scanner_->Throw("Invalid empty path for included file");

    if (! object_stack_.empty())
        dependencies_.push_back(Dependency{ scanner_->GetCurrentPath(), path });

    // If the path is relative, make it absolute.
    if (! Util::FilePath(path).IsAbsolute()) {
        Util::FilePath abs_path = Util::FilePath::GetResourceBasePath();
        abs_path /= path;
        path = abs_path;
    }
    return ParseFile(path);
}

void Parser::ParseConstants_(Object &obj, ConstantsMap_ &map) {
    scanner_->ScanExpectedChar('[');
    while (true) {
        // If the next character is a closing brace, stop. An empty block is
        // valid.
        if (scanner_->PeekChar() == ']')
            break;

        // Parse   name: "string value"
        std::string name = scanner_->ScanName();
        scanner_->ScanExpectedChar(':');
        std::string value = scanner_->ScanQuotedString();

        map[name] = value;

        // Parse the trailing comma.
        if (scanner_->PeekChar() == ',')
            scanner_->ScanExpectedChar(',');
    }
    scanner_->ScanExpectedChar(']');
    // Optional trailing comma after the block.
    if (scanner_->PeekChar() == ',')
        scanner_->ScanExpectedChar(',');
}

const ObjectPtr & Parser::FindObject_(const std::string &type_name,
                                      const std::string &obj_name) {
    auto it = object_name_map_.find(BuildObjectNameKey_(type_name, obj_name));
    if (it == object_name_map_.end())
        scanner_->Throw(std::string("Invalid reference to object of type '") +
                        type_name + "' with name '" + obj_name + "'");
    return it->second;
}

const Parser::ObjectSpec_ & Parser::GetObjectSpec_(
    const std::string &type_name) {
    auto it = object_spec_map_.find(type_name);
    if (it == object_spec_map_.end())
        scanner_->Throw("Unknown object type '" + type_name + "'");
    return it->second;
}

void Parser::ParseFields_(Object &obj, const std::vector<FieldSpec> &specs) {
    while (true) {
        std::string field_name = scanner_->ScanName();
        scanner_->ScanExpectedChar(':');

        // Parse the value(s) and pass them to the storage function.
        const FieldSpec &spec = FindFieldSpec_(obj, specs, field_name);
        ParseAndStoreValues_(obj, spec);

        // Parse the trailing comma.
        char c = scanner_->PeekChar();
        if (c == ',') {
            scanner_->ScanExpectedChar(',');
            c = scanner_->PeekChar();
        }
        // If there was no comma, there must be a closing brace.
        else if (c != '}')
            scanner_->Throw(std::string("Expected ',' or '}', got '") +
                            c + "'");

        // If the next character is a closing brace, stop.
        if (c == '}')
            break;
    }
}

const FieldSpec & Parser::FindFieldSpec_(const Object &obj,
                                         const std::vector<FieldSpec> &specs,
                                         const std::string &field_name) {
    auto it = std::find_if(
        specs.begin(), specs.end(),
        [&](const FieldSpec &s){ return s.name == field_name; });
    if (it == specs.end())
        scanner_->Throw("Unknown field '" + field_name +
                        "' in object of type '" + obj.GetTypeName() + "'");
    return *it;
}

void Parser::ParseAndStoreValues_(Object &obj, const FieldSpec &spec) {
    std::vector<Value> values;
    values.reserve(spec.count);
    for (size_t i = 0; i < spec.count; ++i)
        values.push_back(ParseValue_(spec.type));
    spec.store_func(obj, values);
}

Value Parser::ParseValue_(ValueType type) {
    Value value;
    switch (type) {
      case ValueType::kBool:
        value = scanner_->ScanBool();
        break;
      case ValueType::kInteger:
        value = scanner_->ScanInteger();
        break;
      case ValueType::kFloat:
        value = scanner_->ScanFloat();
        break;
      case ValueType::kString:
        value = scanner_->ScanQuotedString();
        break;
      case ValueType::kObject:
        value = ParseObject_();
        break;
      case ValueType::kObjectList:
        value = ParseObjectList_();
        break;
      default:                                     // LCOV_EXCL_LINE
        scanner_->Throw("Unexpected field type");  // LCOV_EXCL_LINE
    }
    return value;
}

}  // namespace NParser
