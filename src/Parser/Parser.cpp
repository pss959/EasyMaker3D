#include "Parser/Parser.h"

#include <fstream>
#include <functional>
#include <istream>
#include <ranges>
#include <unordered_map>

#include "Parser/ObjectList.h"
#include "Parser/Registry.h"
#include "Parser/Scanner.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

namespace Parser {

// ----------------------------------------------------------------------------
// Parser::Impl_ class.
// ----------------------------------------------------------------------------

class Parser::Impl_ {
  public:
    Impl_();
    ~Impl_();

    /// Sets the directory used to turn relative include paths into absolute
    /// ones. This is FilePath::GetResourceBasePath() by default.
    void SetBasePath(const FilePath &path) { base_path_ = path; }

    /// Parses the contents of the file with the given path, returning the root
    /// Object in the parse graph.
    ObjectPtr ParseFile(const FilePath &path);

    /// Parses the contents of the given string, returning the root Object in
    /// the parse graph.
    ObjectPtr ParseFromString(const Str &str);

    /// Returns a vector of all path dependencies created by included files
    /// found during parsing.
    const std::vector<Dependency> GetDependencies() const {
        return dependencies_;
    }

    /// Resets the state to initial conditions.
    void Reset();

  private:
    /// Convenience typedef for a map storing constants (name -> value).
    typedef std::unordered_map<Str, Str> ConstantsMap_;

    /// Convenience typedef for a map storing objects keyed by some name.
    typedef std::unordered_map<Str, ObjectPtr> ObjectMap_;

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

    /// Base path used to handle relative include file paths.
    FilePath base_path_;

    /// Scanner used to parse tokens.
    std::unique_ptr<Scanner> scanner_;

    /// Stack of ObjectScope_ instances representing current objects being
    /// parsed. This is implemented as an std::vector (rather than an
    /// std::stack) because all scopes need to be accessible for constant
    /// searches.
    std::vector<ObjectScope_> scope_stack_;

    /// Vector of Dependency instances created when a file is included.
    std::vector<Dependency> dependencies_;

    /// This is set to point to the current Template Object when parsing
    /// Templates to distinguish them from regular objects. It is null
    /// otherwise. Templates are not validated.
    ObjectPtr current_template_;

    /// Implements most of ParseFile().
    ObjectPtr ParseFromFile_(const FilePath &path, bool is_template);

    /// Parses the next Object in the input. If is_template is true, the Object
    /// is treated as a Template.
    ObjectPtr ParseObject_(bool is_template);

    /// Parses and returns an object for a USE statement.
    ObjectPtr ParseUse_();

    /// Parses and returns a clone of a template or regular object with
    /// optional field overrides.
    ObjectPtr ParseClone_(bool is_template);

    /// Parses and returns the contents of an Object. The type name is
    /// supplied. If obj_to_clone is not null, the new object should be cloned
    /// from it instead of being created from scratch.
    ObjectPtr ParseObjectContents_(const Str &type_name,
                                   ObjectPtr obj_to_clone, bool is_template);

    /// Parses a collection of Object instances (in square brackets, separated
    /// by commas) from the input, returning a pointer to an ObjectList. If
    /// are_templates is true, the Objects are treated as Templates.
    ObjectListPtr ParseObjectList_(bool are_templates);

    /// Parses the contents of an included file, returning its root Object.
    ObjectPtr ParseIncludedFile_(bool is_template);

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
    ObjectPtr FindObject_(const Str &name, bool can_be_template);

    /// Parses the fields of the given Object, storing values in the instance.
    void ParseFields_(Object &obj);

    /// Function used by Scanner to get the value string to substitute for a
    /// constant with the given name.
    Str SubstituteConstant_(const Str &name) const;

    /// Throws an exception using the Scanner.
    void Throw_(const Str &msg) const;

    /// Builds a name key for an object from its type name and name.
    static Str BuildObjectNameKey_(const Str &obj_type_name,
                                   const Str &obj_name) {
        return obj_type_name + '/' + obj_name;
    }

    /// Returns a string describing the current ObjectScope_ for error
    /// messages.
    Str GetScopeDesc_() const;

    /// Returns a string representing the current ObjectScope_ stack.
    Str GetScopeStackString_() const;
};

// ----------------------------------------------------------------------------
// Parser::Impl_ functions.
// ----------------------------------------------------------------------------

Parser::Impl_::Impl_() :
    base_path_(FilePath::GetResourceBasePath()),
    scanner_(new Scanner([&](const Str &s){
        return SubstituteConstant_(s); })) {
    scanner_->SetObjectFunction([&](){ return ParseObject_(false); });
    scanner_->SetObjectListFunction([&](){ return ParseObjectList_(false); });
}

Parser::Impl_::~Impl_() {
}

ObjectPtr Parser::Impl_::ParseFile(const FilePath &path) {
    scanner_->Clear();
    return ParseFromFile_(path, false);
}

ObjectPtr Parser::Impl_::ParseFromString(const Str &str) {
    scanner_->Clear();
    scanner_->PushStringInput(str);
    ObjectPtr obj = ParseObject_(false);
    scanner_->PopInputStream();
    return obj;
}

void Parser::Impl_::Reset() {
    base_path_.Clear();
    scanner_->Clear();
    scope_stack_.clear();
    dependencies_.clear();
    current_template_.reset();
}

ObjectPtr Parser::Impl_::ParseFromFile_(const FilePath &path,
                                        bool is_template) {
    std::ifstream in(path.ToNativeString());
    if (in.fail())
        Throw_("Failed to open file: " + path.ToString());
    scanner_->PushInputStream(path, in);
    ObjectPtr obj = ParseObject_(is_template);
    scanner_->PopInputStream();
    return obj;
}

ObjectPtr Parser::Impl_::ParseObject_(bool is_template) {
    // Check for an included file: "<...path...>"
    if (scanner_->PeekChar() == '<')
        return ParseIncludedFile_(is_template);

    Str type_name = scanner_->ScanName("object type");

    // Special handling for USE and CLONE keywords.
    ObjectPtr obj;
    if (type_name == "USE")
        obj = ParseUse_();
    else if (type_name == "CLONE")
        obj = ParseClone_(is_template);
    else
        obj = ParseObjectContents_(type_name, ObjectPtr(), is_template);
    return obj;
}

ObjectPtr Parser::Impl_::ParseUse_() {
    // Syntax is:   USE "name"
    //    name must refer to an Object in scope.
    const Str name = scanner_->ScanQuotedString();
    if (name.empty())
        Throw_("Missing Object name for USE in " + GetScopeDesc_());
    ObjectPtr obj = FindObject_(name, false);
    if (! obj)
        Throw_("Missing object with name '" + name +
               "' for USE in " + GetScopeDesc_());
    return obj;
}

ObjectPtr Parser::Impl_::ParseClone_(bool is_template) {
    // Syntax is:   CLONE "name"
    //    name must refer to a Template or Object in scope.
    const Str name = scanner_->ScanQuotedString();
    if (name.empty())
        Throw_("Missing Template or Object name for CLONE in " +
               GetScopeDesc_());
    // Look for a Template or Object.
    ObjectPtr obj = FindObject_(name, true);
    if (! obj)
        Throw_("Missing Template or Object with name '" + name +
               "' for CLONE in " + GetScopeDesc_());
    obj = ParseObjectContents_(obj->GetTypeName(), obj, is_template);
    return obj;
}

ObjectPtr Parser::Impl_::ParseObjectContents_(const Str &type_name,
                                              ObjectPtr obj_to_clone,
                                              bool is_template) {
    // If the next character is a quotation mark, parse the name.
    Str obj_name;
    if (scanner_->PeekChar() == '"')
        obj_name = scanner_->ScanQuotedString();

    // Create the object.
    ObjectPtr obj;
    try {
        obj = obj_to_clone ? obj_to_clone->Clone_(obj_name, true, false) :
            Registry::CreateObjectOfType_(type_name, obj_name, false);
    }
    catch (Exception &ex) {
        // Add context to the generic Registry exception.
        Throw_(ex.what());
    }

    // Check for missing required name, except inside templates.
    if (! is_template && obj->IsNameRequired() && obj_name.empty())
        Throw_("Object of type '" + type_name + " must have a name");

    scanner_->ScanExpectedChar('{');

    KLOG('P', "Parsing contents of " << obj->GetDesc()
         << (obj_to_clone ?
             (" (CLONE of " + obj_to_clone->GetDesc() + ")") : ""));

    if (obj->IsScoped())
        PushScope_(obj);

    if (scanner_->PeekChar() != '}')  // Valid to have an object with no fields.
        ParseFields_(*obj);

    scanner_->ScanExpectedChar('}');

    // Let the derived class check for errors. Do not validate Templates.
    if (! is_template) {
        Str details;
        if (! obj->IsValid(details))
            Throw_(obj->GetDesc() + " has error: " + details);
    }

    // The instance is now complete.
    obj->CompleteInstance_(is_template ? Object::InstanceType_::kTemplate :
                           obj_to_clone ? Object::InstanceType_::kClone :
                           Object::InstanceType_::kRegular);

    // Pop the scope so the parent's scope (if any) is now current. If the new
    // Object has a name, store it in the parent's scope.
    if (obj->IsScoped())
        PopScope_(obj);
    if (! obj_name.empty() && ! scope_stack_.empty()) {
        scope_stack_.back().objects_map[obj_name] = obj;
        KLOG('s', " Stored " << obj->GetDesc() << " in current scope");
    }

    return obj;
}

ObjectListPtr Parser::Impl_::ParseObjectList_(bool are_templates) {
    ObjectListPtr list(new ObjectList);
    scanner_->ScanExpectedChar('[');
    while (true) {
        // If the next character is a closing brace, stop. An empty list of
        // objects is valid.
        if (scanner_->PeekChar() == ']')
            break;

        list->objects.push_back(ParseObject_(are_templates));

        // Parse the trailing comma.
        if (scanner_->PeekChar() == ',')
            scanner_->ScanExpectedChar(',');
    }
    scanner_->ScanExpectedChar(']');
    return list;
}

ObjectPtr Parser::Impl_::ParseIncludedFile_(bool is_template) {
    scanner_->ScanExpectedChar('<');
    Str path = scanner_->ScanQuotedString();
    scanner_->ScanExpectedChar('>');
    if (path.empty())
        Throw_("Invalid empty path for included file");

    if (! scope_stack_.empty())
        dependencies_.push_back(Dependency{ scanner_->GetCurrentPath(), path });

    // If the path is relative, make it absolute.
    FilePath fp(path);
    if (! fp.IsAbsolute())
        fp = FilePath::Join(base_path_, fp);
    return ParseFromFile_(fp, is_template);
}

ObjectPtr Parser::Impl_::FindObject_(const Str &name, bool can_be_template) {
    // Look in all open scopes, starting at the top of the stack (reverse
    // iteration).
    for (const auto &scope: scope_stack_ | std::views::reverse) {
        if (can_be_template) {
            auto ti = scope.templates_map.find(name);
            if (ti != scope.templates_map.end()) {
                KLOG('s', " Found Template " << ti->second->GetDesc()
                     << " in current scope");
                return ti->second;
            }
        }
        auto oi = scope.objects_map.find(name);
        if (oi != scope.objects_map.end()) {
            KLOG('s', " Found Object " << oi->second->GetDesc()
                 << " in current scope");
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
        Str field_name = scanner_->ScanName("field name");
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
            Throw_(Str("Expected ',' or '}', got '") + c + "'");
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
    KLOG('s', "Pushed scope, now " << GetScopeStackString_());

}

void Parser::Impl_::PopScope_(const ObjectPtr &obj) {
    ASSERT(! scope_stack_.empty());
    ASSERT(scope_stack_.back().object == obj);
    scope_stack_.pop_back();
    KLOG('s', "Popped scope, now " << GetScopeStackString_());
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
        Str name = scanner_->ScanName("constant name");
        scanner_->ScanExpectedChar(':');
        Str value = scanner_->ScanQuotedString();

        scope.constants_map[name] = value;

        // Parse the trailing comma.
        if (scanner_->PeekChar() == ',')
            scanner_->ScanExpectedChar(',');
    }
    scanner_->ScanExpectedChar(']');
}

void Parser::Impl_::ParseTemplates_() {
    // Parse the Template definitions as Objects. Pass true to indicate that
    // templates are being parsed (as opposed to regular Objects).
    auto list = ParseObjectList_(true);
    ASSERT(list);

    // Add the Templates to the current scope.
    ASSERT(! scope_stack_.empty());
    auto &scope = scope_stack_.back();
    for (auto &obj: list->objects)
        scope.templates_map[obj->GetName()] = obj;
}

Str Parser::Impl_::SubstituteConstant_(const Str &name) const {
    // Look up the Constant in all active scopes, starting at the top (reverse
    // iteration).
    for (const auto &scope: scope_stack_ | std::views::reverse) {
        auto ci = scope.constants_map.find(name);
        if (ci != scope.constants_map.end())
            return ci->second;
    }
    // If we get here, the constant was not found.
    Throw_("Missing constant with name '" + name + "'");
    return "";  // LCOV_EXCL_LINE [cannot happen]
}

void Parser::Impl_::Throw_(const Str &msg) const {
    scanner_->Throw(msg);
}

Str Parser::Impl_::GetScopeDesc_() const {
    if (scope_stack_.empty())
        return "Top-level scope";
    else
        return "Scope " + GetScopeStackString_();
}

Str Parser::Impl_::GetScopeStackString_() const {
    Str s;
    for (const auto &scope: scope_stack_) {
        if (! s.empty())
            s += "/";
        const Str n = scope.object->GetName();
        s += n.empty() ? "*" : n;
    }
    return s;
}

// ----------------------------------------------------------------------------
// Parser functions.
// ----------------------------------------------------------------------------

Parser::Parser() : impl_(new Impl_) {
}

Parser::~Parser() {
}

void Parser::SetBasePath(const FilePath &path) {
    impl_->SetBasePath(path);
}

ObjectPtr Parser::ParseFile(const FilePath &path) {
    return impl_->ParseFile(path);
}

ObjectPtr Parser::ParseFromString(const Str &str) {
    return impl_->ParseFromString(str);
}

const std::vector<Parser::Dependency> Parser::GetDependencies() const {
    return impl_->GetDependencies();
}

void Parser::Reset() {
    impl_->Reset();
}

}  // namespace Parser
