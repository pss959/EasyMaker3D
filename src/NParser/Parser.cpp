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

void Parser::RegisterObject(const std::string &type_name,
                            const CreationFunc &creation_func) {
    assert(! Util::MapContains(object_creation_map_, type_name));
    object_creation_map_[type_name] = creation_func;
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

    ObjectPtr obj = CreateObject_(type_name);
    obj->SetName_(obj_name);
    object_stack_.push_back(obj);
    scanner_->ScanExpectedChar('{');

    // Check for constants block as the first thing.
    /* XXXX
    if (scanner_->PeekChar() == '[')
        ParseConstants_(*obj);
    */

    if (scanner_->PeekChar() != '}')  // Valid to have an object with no fields.
        ParseFields_(*obj);
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

ObjectPtr Parser::CreateObject_(const std::string &type_name) {
    auto it = object_creation_map_.find(type_name);
    if (it == object_creation_map_.end())
        scanner_->Throw("Unknown object type '" + type_name + "'");
    // Invoke the creation function.
    ObjectPtr obj(it->second());
    obj->SetTypeName_(type_name);
    return obj;
}

void Parser::ParseFields_(Object &obj) {
    const FieldSpecs &specs = obj.GetFieldSpecs();

    while (true) {
        std::string field_name = scanner_->ScanName();
        scanner_->ScanExpectedChar(':');

        const FieldSpecs::Spec *spec = FindFieldSpec_(specs, field_name);
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

const FieldSpecs::Spec * Parser::FindFieldSpec_(const FieldSpecs &specs,
                                                const std::string &field_name) {
    auto it = std::find_if(
        specs.specs.begin(), specs.specs.end(),
        [&](const FieldSpecs::Spec &s){ return s.name == field_name; });
    return it == specs.specs.end() ? nullptr : &(*it);
}

void Parser::ParseAndStoreValues_(Object &obj, const FieldSpecs::Spec &spec) {
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
