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
    /* XXXX
    // Check for an included file: "<...path...>"
    if (scanner_->PeekChar() == '<')
        return ParseIncludedFile_();
    */

    std::string type_name = scanner_->ScanName();

    // If the next character is a quotation mark, parse the name.
    std::string obj_name;
    if (scanner_->PeekChar() == '"') {
        obj_name = scanner_->ScanQuotedString();

        // If the next character is a ';', this is a reference to an existing
        // object.
        if (scanner_->PeekChar() == ';') {
            scanner_->ScanExpectedChar(';');
            // XXXX return GetObjectByName_(obj_name);
        }
    }

    // Get the ObjectSpec_ for the type of object.
    const ObjectSpec_ &spec = GetObjectSpec_(type_name);

    // Invoke the creation function.
    ObjectPtr obj(spec.creation_func());
    obj->SetTypeName_(type_name);
    obj->SetName_(obj_name);
    object_stack_.push_back(obj);
    scanner_->ScanExpectedChar('{');

    // Check for constants block as the first thing.
    /* XXXX
    if (scanner_->PeekChar() == '[')
        ParseConstants_(*obj);
    */

    if (scanner_->PeekChar() != '}')  // Valid to have an object with no fields.
        ParseFields_(*obj, spec.field_specs);
    scanner_->ScanExpectedChar('}');
    assert(object_stack_.back() == obj);
    object_stack_.pop_back();

    // If there was a name given, store it in the map.
    /* XXXX
    if (! obj_name.empty()) {
        obj->name = obj_name;
        const std::string qual_name =
            GetQualifiedObjectName_(spec.type_name, obj_name);
        object_name_map_[qual_name] = obj;
    }
    */

    return obj;
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

        const FieldSpec *spec = FindFieldSpec_(specs, field_name);
        if (! spec)
            scanner_->Throw("Unknown field '" + field_name +
                            "' in object of type '" + obj.GetTypeName() + "'");

        // Parse the value(s) and pass them to the storage function.
        ParseAndStoreValues_(obj, *spec);

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

const FieldSpec * Parser::FindFieldSpec_(const std::vector<FieldSpec> &specs,
                                         const std::string &field_name) {
    auto it = std::find_if(
        specs.begin(), specs.end(),
        [&](const FieldSpec &s){ return s.name == field_name; });
    return it == specs.end() ? nullptr : &(*it);
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
        // value = ParseObjectList_();
        // XXXX
        break;
      default:                                     // LCOV_EXCL_LINE
        scanner_->Throw("Unexpected field type");  // LCOV_EXCL_LINE
    }
    return value;
}

}  // namespace NParser
