#include "Parser/Parser.h"

#include <fstream>
#include <functional>
#include <istream>
#include <unordered_map>

#include "Parser/ObjectList.h"
#include "Parser/Registry.h"
#include "Parser/Scanner.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"

namespace Parser {

// ----------------------------------------------------------------------------
// Parser::Impl_ class.
// ----------------------------------------------------------------------------

class Parser::Impl_ {
  public:
    Impl_();
    ~Impl_();

    /// Parses the contents of the file with the given path, returning the root
    /// Object in the parse graph.
    ObjectPtr ParseFile(const Util::FilePath &path);

    /// Parses the contents of the given string, returning the root Object in
    /// the parse graph.
    ObjectPtr ParseFromString(const std::string &str);

    /// Returns a vector of all path dependencies created by included files
    /// found during parsing.
    const std::vector<Dependency> GetDependencies() const {
        return dependencies_;
    }

  private:
    /// Convenience typedef for a map storing constants (name -> value).
    typedef std::unordered_map<std::string, std::string> ConstantsMap_;

    /// Convenience typedef for a map storing objects keyed by some name.
    typedef std::unordered_map<std::string, ObjectPtr> ObjectMap_;

    /// This struct is stored in the object_stack_. It maintains a pointer to
    /// the Object and everything scoped by it:
    /// \li Constants defined in the scope, mapped from constant name to the
    ///     value string.
    /// \li Templates defined in the scope, mapped from template name to the
    ///     Object.
    /// \li All real Objects defined within the scope, keyed by Object name.
    struct ObjectScope_ {
        ObjectPtr     object;
        ConstantsMap_ constants_map;
        ObjectMap_    templates_map;
        ObjectMap_    objects_map;
    };

    /// Scanner used to parse tokens.
    std::unique_ptr<Scanner> scanner_;

    /// Stack of ObjectScope_ instances representing current objects being
    /// parsed. This is implemented as an std::vector (rather than an
    /// std::stack) hbecause all scopes need to be accessible for constant
    /// searches.
    std::vector<ObjectScope_> scope_stack_;

    /// Vector of Dependency instances created when a file is included.
    std::vector<Dependency> dependencies_;

    /// This is set to true when parsing Templates to distinguish them from
    /// regular objects - Templates are not validated.
    bool is_parsing_templates_ = false;

    /// Implements most of ParseFile().
    ObjectPtr ParseFromFile_(const Util::FilePath &path);

    /// Parses the next Object in the input.
    ObjectPtr ParseObject_();

    /// Parses and returns an object for a USE statement.
    ObjectPtr ParseUse_();

    /// Parses and returns a clone of a template or regular object with
    /// optional field overrides.
    ObjectPtr ParseClone_();

    /// Parses and returns the contents of an Object. The type name is
    /// supplied. If obj_to_clone is not null, the new object should be cloned
    /// from it instead of being created from scratch.
    ObjectPtr ParseObjectContents_(const std::string &type_name,
                                   ObjectPtr obj_to_clone);

    /// Parses a collection of Object instances (in square brackets, separated
    /// by commas) from the input, returning a pointer to an ObjectList.
    ObjectListPtr ParseObjectList_();

    /// Parses the contents of an included file, returning its root Object.
    ObjectPtr ParseIncludedFile_();

    /// Pushes a scope for the given Object onto the stack.
    void PushScope_(const ObjectPtr &obj);

    /// Pops the top scope from the stack, verifying that it is for the given
    /// Object.
    void PopScope_(const ObjectPtr &obj);

    /// Parses constant definitions, storing them in the current scope's map.
    void ParseConstants_();

    /// Parses Templates, storing them in the current scope's map.
    void ParseTemplates_();

    /// Looks through all active scopes for an Object with the given name.
    /// If can_be_template is true, Templates are searched as well.
    /// Returns null if not found.
    ObjectPtr FindObject_(const std::string &name, bool can_be_template);

    /// Parses the fields of the given Object, storing values in the instance.
    void ParseFields_(Object &obj);

    /// Function used by Scanner to get the value string to substitute for a
    /// constant with the given name.
    std::string SubstituteConstant_(const std::string &name) const;

    /// Throws an exception using the Scanner.
    void Throw_(const std::string &msg) const;

    /// Builds a name key for an object from its type name and name.
    static std::string BuildObjectNameKey_(const std::string &obj_type_name,
                                           const std::string &obj_name) {
        return obj_type_name + '/' + obj_name;
    }

    /// Dumps the ObjectScope_ stack to standard out for help in debugging.
    void DumpScopeStack_() const;
};

// ----------------------------------------------------------------------------
// Parser::Impl_ functions.
// ----------------------------------------------------------------------------

Parser::Impl_::Impl_() : scanner_(
    new Scanner(std::bind(&Impl_::SubstituteConstant_, this,
                          std::placeholders::_1))) {
    scanner_->SetObjectFunction(std::bind(&Impl_::ParseObject_, this));
    scanner_->SetObjectListFunction(std::bind(&Impl_::ParseObjectList_, this));
}

Parser::Impl_::~Impl_() {
}

ObjectPtr Parser::Impl_::ParseFile(const Util::FilePath &path) {
    scanner_->Clear();
    return ParseFromFile_(path);
}

ObjectPtr Parser::Impl_::ParseFromString(const std::string &str) {
    scanner_->Clear();
    scanner_->PushStringInput(str);
    ObjectPtr obj = ParseObject_();
    scanner_->PopInputStream();
    return obj;
}

ObjectPtr Parser::Impl_::ParseFromFile_(const Util::FilePath &path) {
    std::ifstream in(path.ToNativeString());
    if (in.fail())
        Throw_("Failed to open file: " + path.ToString());
    scanner_->PushInputStream(path, in);
    ObjectPtr obj = ParseObject_();
    scanner_->PopInputStream();
    return obj;
}

ObjectPtr Parser::Impl_::ParseObject_() {
    // Check for an included file: "<...path...>"
    if (scanner_->PeekChar() == '<')
        return ParseIncludedFile_();

    std::string type_name = scanner_->ScanName("object type");

    // Special handling for USE and CLONE keywords.
    ObjectPtr obj;
    if (type_name == "USE")
        obj = ParseUse_();
    else if (type_name == "CLONE")
        obj = ParseClone_();
    else
        obj = ParseObjectContents_(type_name, ObjectPtr());
    return obj;
}

ObjectPtr Parser::Impl_::ParseUse_() {
    // Syntax is:   USE "name"
    //    name must refer to an Object in scope.
    const std::string name = scanner_->ScanQuotedString();
    if (name.empty())
        Throw_("Missing Object name for USE");
    ObjectPtr obj = FindObject_(name, false);
    if (! obj)
        Throw_("Missing object with name '" + name + "' for USE");
    return obj;
}

ObjectPtr Parser::Impl_::ParseClone_() {
    // Syntax is:   CLONE "name"
    //    name must refer to a Template or Object in scope.
    const std::string name = scanner_->ScanQuotedString();
    if (name.empty())
        Throw_("Missing Template or Object name for CLONE");
    // Look for a Template or Object.
    ObjectPtr obj = FindObject_(name, true);
    if (! obj)
        Throw_("Missing Template or Object with name '" + name + "' for CLONE");
    obj = ParseObjectContents_(obj->GetTypeName(), obj);
    obj->SetIsClone();
    return obj;
}

ObjectPtr Parser::Impl_::ParseObjectContents_(const std::string &type_name,
                                       ObjectPtr obj_to_clone) {
    // If the next character is a quotation mark, parse the name.
    std::string obj_name;
    if (scanner_->PeekChar() == '"')
        obj_name = scanner_->ScanQuotedString();

    // Create the object.
    ObjectPtr obj;
    try {
        obj = obj_to_clone ? obj_to_clone->Clone(true) :
            Registry::CreateObjectOfType(type_name);
    }
    catch (Exception &ex) {
        // Add context to the generic Registry exception.
        Throw_(ex.what());
    }

    // Check for missing required name.
    if (obj->IsNameRequired() && obj_name.empty())
        Throw_("Object of type '" + type_name + " must have a name");

    obj->SetName(obj_name);
    obj->ConstructionDone(); // Now that name is set.
    scanner_->ScanExpectedChar('{');

    PushScope_(obj);

    if (scanner_->PeekChar() != '}')  // Valid to have an object with no fields.
        ParseFields_(*obj);

    scanner_->ScanExpectedChar('}');

    // Let the derived class check for errors. Do not validate Templates.
    if (! is_parsing_templates_) {
        std::string details;
        if (! obj->IsValid(details))
            Throw_(obj->GetDesc() + " has error: " + details);
    }

    // Pop the scope so the parent's scope (if any) is now current. If the new
    // Object has a name, store it in the parent's scope.
    PopScope_(obj);
    if (! obj_name.empty() && ! scope_stack_.empty()) {
        scope_stack_.back().objects_map[obj_name] = obj;
        KLOG('o', "Stored " << obj->GetDesc() << " in scope of "
             << scope_stack_.back().object->GetDesc());
    }

    return obj;
}

#if XXXX
ObjectPtr Parser::Impl_::AddTemplate_() {
    // Parse the template object type name.
    std::string type_name = scanner_->ScanName("template object type");

    // Parse the object normally, but indicate that it is a template.
    ObjectPtr obj = ParseRegularObject_(type_name, true, ObjectPtr());
    obj->SetObjectType(Object::ObjType::kTemplate);

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
#endif

ObjectListPtr Parser::Impl_::ParseObjectList_() {
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

ObjectPtr Parser::Impl_::ParseIncludedFile_() {
    scanner_->ScanExpectedChar('<');
    std::string path = scanner_->ScanQuotedString();
    scanner_->ScanExpectedChar('>');
    if (path.empty())
        Throw_("Invalid empty path for included file");

    if (! scope_stack_.empty())
        dependencies_.push_back(Dependency{ scanner_->GetCurrentPath(), path });

    // If the path is relative, make it absolute.
    Util::FilePath fp(path);
    if (! fp.IsAbsolute())
        fp = Util::FilePath::Join(Util::FilePath::GetResourceBasePath(), fp);
    return ParseFromFile_(fp);
}

ObjectPtr Parser::Impl_::FindObject_(const std::string &name,
                                     bool can_be_template) {
    // Look in all open scopes, starting at the top of the stack (reverse
    // iteration).
    for (auto it = std::rbegin(scope_stack_);
         it != std::rend(scope_stack_); ++it) {
        const auto &scope = *it;
        if (can_be_template) {
            auto ti = scope.templates_map.find(name);
            if (ti != scope.templates_map.end()) {
                KLOG('o', "Found Template " << ti->second->GetDesc()
                     << " in scope of " << scope.object->GetDesc());
                return ti->second;
            }
        }
        auto oi = scope.objects_map.find(name);
        if (oi != scope.objects_map.end()) {
            KLOG('o', "Found Object " << oi->second->GetDesc()
                 << " in scope of " << scope.object->GetDesc());
            return oi->second;
        }
    }
    // Not found.
    return ObjectPtr();
}

void Parser::Impl_::ParseFields_(Object &obj) {
    // This is used to ensure that CONSTANTS and TEMPLATES fields are parsed
    // before any real fields.
    bool any_real_field_parsed = false;

    while (true) {
        std::string field_name = scanner_->ScanName("field name");
        scanner_->ScanExpectedChar(':');

        // Special cases.
        if (field_name == "CONSTANTS") {
            if (any_real_field_parsed)
                Throw_("CONSTANTS appears after fields in " + obj.GetDesc());
            ParseConstants_();
        }
        else if (field_name == "TEMPLATES") {
            if (any_real_field_parsed)
                Throw_("TEMPLATES appears after fields in " + obj.GetDesc());
            ParseTemplates_();
        }

        else {
            // Look for the field with the given name and read its value.
            Field * field = obj.FindField(field_name);
            if (! field)
                Throw_("Unknown field '" + field_name +
                       "' in object of type '" + obj.GetTypeName() + "'");
            field->ParseValue(*scanner_);
            field->SetWasSet(true);
            obj.SetFieldParsed(*field);
            any_real_field_parsed = true;
        }

        // Parse the trailing comma, if any.
        char c = scanner_->PeekChar();
        if (c == ',') {
            scanner_->ScanExpectedChar(',');
            c = scanner_->PeekChar();
        }
        // If there was no comma, there must be a closing brace.
        else if (c != '}') {
            Throw_(std::string("Expected ',' or '}', got '") + c + "'");
        }

        // If the next character is a closing brace, stop.
        if (c == '}')
            break;
    }
}

void Parser::Impl_::PushScope_(const ObjectPtr &obj) {
    ObjectScope_ scope;
    scope.object = obj;
    scope_stack_.push_back(scope);
}

void Parser::Impl_::PopScope_(const ObjectPtr &obj) {
    ASSERT(! scope_stack_.empty());
    ASSERT(scope_stack_.back().object == obj);
    scope_stack_.pop_back();
}

void Parser::Impl_::ParseConstants_() {
    ASSERT(! scope_stack_.empty());
    auto &scope = scope_stack_.back();

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

        scope.constants_map[name] = value;

        // Parse the trailing comma.
        if (scanner_->PeekChar() == ',')
            scanner_->ScanExpectedChar(',');
    }
    scanner_->ScanExpectedChar(']');
}

void Parser::Impl_::ParseTemplates_() {
    // Parse the Template definitions as Objects. Set the flag to indicate that
    // templates are being parsed (as opposed to regular Objects).
    is_parsing_templates_ = true;
    auto list = ParseObjectList_();
    ASSERT(list);
    is_parsing_templates_ = false;

    // Add the Templates to the current scope.
    ASSERT(! scope_stack_.empty());
    auto &scope = scope_stack_.back();
    for (auto &obj: list->objects)
        scope.templates_map[obj->GetName()] = obj;
}

std::string Parser::Impl_::SubstituteConstant_(const std::string &name) const {
    // Look up the Constant in all active scopes, starting at the top (reverse
    // iteration).
    for (auto it = std::rbegin(scope_stack_);
         it != std::rend(scope_stack_); ++it) {
        const auto &scope = *it;
        auto ci = scope.constants_map.find(name);
        if (ci != scope.constants_map.end())
            return ci->second;
    }
    // If we get here, the constant was not found.
    Throw_("Missing constant with name '" + name + "'");
    return "";  // LCOV_EXCL_LINE
}

void Parser::Impl_::Throw_(const std::string &msg) const {
    scanner_->Throw(msg);
}

void Parser::Impl_::DumpScopeStack_() const {
    std::cerr << "=== Parser object stack (bottom to top):\n";
    int level = 0;
    for (const auto &scope: scope_stack_)
        std::cerr << "[" << level++ << "] " << scope.object->GetDesc() << "\n";
}

// ----------------------------------------------------------------------------
// Parser functions.
// ----------------------------------------------------------------------------

Parser::Parser() : impl_(new Impl_) {
}

Parser::~Parser() {
}

ObjectPtr Parser::ParseFile(const Util::FilePath &path) {
    return impl_->ParseFile(path);
}

ObjectPtr Parser::ParseFromString(const std::string &str) {
    return impl_->ParseFromString(str);
}

const std::vector<Parser::Dependency> Parser::GetDependencies() const {
    return impl_->GetDependencies();
}

}  // namespace Parser
