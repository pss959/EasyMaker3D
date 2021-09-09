#include "Parser/Parser.h"

#include <fstream>

#include "Assert.h"
#include "Parser/ObjectList.h"
#include "Parser/Scanner.h"
#include "Util/General.h"

namespace Parser {

Parser::Parser() : scanner_(
    new Scanner(std::bind(&Parser::SubstituteConstant_, this,
                          std::placeholders::_1))) {
    scanner_->SetObjectFunction(std::bind(&Parser::ParseObject_, this));
    scanner_->SetObjectListFunction(std::bind(&Parser::ParseObjectList_, this));
}

Parser::~Parser() {
}

void Parser::RegisterObjectType(const std::string &type_name,
                                const CreationFunc &creation_func) {
    if (Util::MapContains(object_func_map_, type_name))
        Throw_("Object type registered more than once: '" + type_name + "'");
    object_func_map_[type_name] = creation_func;
}

ObjectPtr Parser::ParseFile(const Util::FilePath &path) {
    scanner_->Clear();
    return ParseFromFile_(path);
}

ObjectPtr Parser::ParseFromString(const std::string &str) {
    scanner_->Clear();
    scanner_->PushStringInput(str);
    ObjectPtr obj = ParseObject_();
    scanner_->PopInputStream();
    return obj;
}

ObjectPtr Parser::ParseFromFile_(const Util::FilePath &path) {
    std::ifstream in(path);
    if (in.fail())
        Throw_("Failed to open file");
    scanner_->PushInputStream(path, in);
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

    // Create an object of the correct type using the CreationFunc.
    ObjectPtr obj = CreateObjectOfType_(type_name);

    // Check for missing required name.
    if (obj->IsNameRequired() && obj_name.empty())
        Throw_("Object of type '" + type_name + " must have a name");

    obj->SetName(obj_name);
    obj->ConstructionDone(); // Now that name is set.
    scanner_->ScanExpectedChar('{');

    // Create an ObjectData_ instance for the object and add constants to it,
    // if there are any.
    ObjectData_ data;
    data.object = obj;
    if (scanner_->PeekChar() == '[')
        ParseConstants_(*obj, data.constants_map);

    object_stack_.push_back(data);

    if (scanner_->PeekChar() != '}')  // Valid to have an object with no fields.
        ParseFields_(*obj);

    scanner_->ScanExpectedChar('}');
    ASSERT(object_stack_.back().object == obj);
    object_stack_.pop_back();

    // If the object has a name, store it in the map.
    if (! obj_name.empty())
        object_name_map_[BuildObjectNameKey_(type_name, obj_name)] = obj;

    return obj;
}

ObjectListPtr Parser::ParseObjectList_() {
    ObjectListPtr list(new ObjectList);
    scanner_->ScanExpectedChar('[');
    while (true) {
        // If the next character is a closing brace, stop. An empty list of
        // objects is valid.
        if (scanner_->PeekChar() == ']')
            break;

        list->objects.push_back(ParseObject_());

        // Parse the trailing comma.
        char c = scanner_->PeekChar();
        if (c == ',')
            scanner_->ScanExpectedChar(',');
    }
    scanner_->ScanExpectedChar(']');
    return list;
}

std::vector<ObjectPtr> Parser::ParseObjectList2_() {
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
        Throw_("Invalid empty path for included file");

    if (! object_stack_.empty())
        dependencies_.push_back(Dependency{ scanner_->GetCurrentPath(), path });

    // If the path is relative, make it absolute.
    if (! Util::FilePath(path).IsAbsolute()) {
        Util::FilePath abs_path = Util::FilePath::GetResourceBasePath();
        abs_path /= path;
        path = abs_path;
    }
    return ParseFromFile_(path);
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
        Throw_(std::string("Invalid reference to object of type '") +
                        type_name + "' with name '" + obj_name + "'");
    return it->second;
}

ObjectPtr Parser::CreateObjectOfType_(const std::string &type_name) {
    // Look up and call the CreationFunc.
    auto it = object_func_map_.find(type_name);
    if (it == object_func_map_.end())
        Throw_("Unknown object type '" + type_name + "'");
    const CreationFunc &creation_func = it->second;

    // Call it, then tell the object to set up fields for parsing.
    ObjectPtr obj(creation_func());
    obj->SetTypeName_(type_name);
    obj->AddFields();
    return obj;
}

void Parser::ParseFields_(Object &obj) {
    while (true) {
        std::string field_name = scanner_->ScanName();
        scanner_->ScanExpectedChar(':');

        // Look for the field with the given name and read its value.
        Field * field = obj.FindField(field_name);
        if (! field)
            Throw_("Unknown field '" + field_name +
                   "' in object of type '" + obj.GetTypeName() + "'");
        field->ParseValue(*scanner_);
        field->SetWasSet(true);
        obj.SetFieldParsed(*field);

        // Parse the trailing comma.
        char c = scanner_->PeekChar();
        if (c == ',') {
            scanner_->ScanExpectedChar(',');
            c = scanner_->PeekChar();
        }
        // If there was no comma, there must be a closing brace.
        else if (c != '}')
            Throw_(std::string("Expected ',' or '}', got '") +
                            c + "'");

        // If the next character is a closing brace, stop.
        if (c == '}')
            break;
    }
}

std::string Parser::SubstituteConstant_(const std::string &name) const {
    // Look up the name in all open objects, starting at the top of the stack
    // (reverse iteration).
    for (auto it = std::rbegin(object_stack_);
         it != std::rend(object_stack_); ++it) {
        const ObjectData_ &obj = *it;
        auto cit = obj.constants_map.find(name);
        if (cit != obj.constants_map.end())
            return cit->second;
    }
    // If we get here, the constant was not found.
    Throw_("Undefined constant '" + name + "'");
    return "";  // LCOV_EXCL_LINE
}

void Parser::Throw_(const std::string &msg) const {
    scanner_->Throw(msg);
}

}  // namespace Parser
