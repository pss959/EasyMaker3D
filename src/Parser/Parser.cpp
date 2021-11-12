#include "Parser/Parser.h"

#include <fstream>

#include "Assert.h"
#include "Parser/ObjectList.h"
#include "Parser/Registry.h"
#include "Parser/Scanner.h"
#include "Util/General.h"
#include "Util/KLog.h"

namespace Parser {

Parser::Parser() : scanner_(
    new Scanner(std::bind(&Parser::SubstituteConstant_, this,
                          std::placeholders::_1))) {
    scanner_->SetObjectFunction(std::bind(&Parser::ParseObject_, this));
    scanner_->SetObjectListFunction(std::bind(&Parser::ParseObjectList_, this));
}

Parser::~Parser() {
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

    std::string type_name = scanner_->ScanName("object type");

    // Special handling for templates and template clones.
    ObjectPtr obj;
    if (type_name == "TEMPLATE")
        obj = AddTemplate_();
    else if (type_name == "CLONE")
        obj = AddClone_();
    else
        obj = ParseRegularObject_(type_name, false, ObjectPtr());

    return obj;
}

ObjectPtr Parser::AddTemplate_() {
    // Parse the template object type name.
    std::string type_name = scanner_->ScanName("template object type");

    // Parse the object normally, but indicate that it is a template.
    ObjectPtr obj = ParseRegularObject_(type_name, true, ObjectPtr());

    // Templates must be named.
    const std::string &name = obj->GetName();
    if (name.empty())
        Throw_("Template Object " + obj->GetDesc() + " must have a name");

    // Check for uniqueness of template name.
    if (Util::MapContains(template_map_, name))
        Throw_("Multiple templates with same name '" + name  + "'");

    template_map_[name] = obj;
    return obj;
}

ObjectPtr Parser::AddClone_() {
    // Parse the template name.
    const std::string template_name = scanner_->ScanQuotedString();
    if (template_name.empty())
        Throw_("Missing template name for clone");

    auto it = template_map_.find(template_name);
    if (it == template_map_.end())
        Throw_("Unknown template '" + template_name + "' for clone");

    ObjectPtr base_obj = it->second;
    return ParseRegularObject_(base_obj->GetTypeName(), false, base_obj);
}

ObjectPtr Parser::ParseRegularObject_(const std::string &type_name,
                                      bool is_template, ObjectPtr base_obj) {
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

    // Create the object.
    ObjectPtr obj = base_obj ? base_obj->Clone(true) :
        Registry::CreateObjectOfType(type_name);
    obj->SetIsTemplate(is_template);

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

    // Let the derived class check for errors.
    std::string details;
    if (! obj->IsValid(details))
        Throw_(obj->GetDesc() + " has error: " + details);

    ASSERT(object_stack_.back().object == obj);
    object_stack_.pop_back();

    // If the object has a name, store it in the scoped map (for the parent,
    // since the stack was just popped) and in the global map.
    if (! obj_name.empty()) {
        const std::string key = BuildObjectNameKey_(type_name, obj_name);
        if (! object_stack_.empty()) {
            object_stack_.back().scoped_objects_map[key] = obj;
            KLOG('o', "Stored " << obj->GetDesc() << " in scope of "
                 << object_stack_.back().object->GetDesc());
        }

        // Store in the global map if this is the first object with that key.
        if (! Util::MapContains(object_name_map_, key))
            object_name_map_[key] = obj;
    }

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
        if (scanner_->PeekChar() == ',')
            scanner_->ScanExpectedChar(',');
    }
    scanner_->ScanExpectedChar(']');
    return list;
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
        std::string name = scanner_->ScanName("constant name");
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
    const std::string key = BuildObjectNameKey_(type_name, obj_name);

    // Look first in scopes in all open objects, starting at the top of the
    // stack (reverse iteration).
    for (auto it = std::rbegin(object_stack_);
         it != std::rend(object_stack_); ++it) {
        const ObjectData_ &obj = *it;
        auto oit = obj.scoped_objects_map.find(key);
        if (oit != obj.scoped_objects_map.end()) {
            KLOG('o', "Found  " << oit->second->GetDesc()
                 << " in scope of " << obj.object->GetDesc());
            return oit->second;
        }
    }

    // If not found, look in the global map. Not finding it there is an error.
    auto it = object_name_map_.find(key);
    if (it == object_name_map_.end())
        Throw_(std::string("Invalid reference to object of type '") +
               type_name + "' with name '" + obj_name + "'");
    KLOG('o', "Found " << it->second->GetDesc() << " in global scope");
    return it->second;
}

void Parser::ParseFields_(Object &obj) {
    while (true) {
        std::string field_name = scanner_->ScanName("field name");
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

void Parser::DumpObjectStack_() const {
    std::cerr << "=== Parser object stack (bottom to top):\n";
    int level = 0;
    for (const auto &data: object_stack_)
        std::cerr << "[" << level++ << "] " << data.object->GetDesc() << "\n";
 }

}  // namespace Parser
