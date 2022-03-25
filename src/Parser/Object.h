#pragma once

#include <string>
#include <vector>

#include "Memory.h"
#include "Parser/Field.h"
#include "Util/Assert.h"
#include "Util/General.h"

namespace Parser {

DECL_SHARED_PTR(Object);

/// Parser::Object is an abstract base class for all objects resulting from
/// parsing. All classes derived from this typically contain Parser::Field
/// instances that allow class-specific data to be read and stored. Derived
/// classes must add these fields after construction so the Parser can access
/// them; the virtual AddFields() function is used to do this.
///
/// There are four ways in which Object instances are created:
///   -# Created directly using types registered with Parser::Registry.
///   -# Cloned from an existing Object instance.
///   -# Parsed from a file with Parser::Parser.
///   -# Cloned during parsing with a "CLONE" statement.
///
/// In all four cases, the following virtual functions are called for the
/// new Object instance:
///   - ConstructionDone() right after the class constructor completes.
///   - AddFields() after the name and type name are set.
///   - CopyContentsFrom() to copy field and other values to the clone (cases 2
///     and 4 only).
///   - IsValid() after parsing is done for the instance (cases 3 and 4 only).
///   - CreationDone() when the Object instance is complete (unless IsValid()
///     fails).
///
/// Note an Object instance created directly using the Registry (case 1) will
/// have all default values for its fields.
///
/// As a result of this setup, the following requirements are placed on all
/// derived Object classes:
///   - If the class has a constructor, it must be protected. (Instances cannot
///     be created by anyone other than the Parser::Registry.)
///   - If the class is concrete, it must be registered with the
///     Parser::Registry so that instances may be created by the Parser.
///   - The class should add Parser::Registry as a friend so that the registry
///     can create instances of it.
class Object {
  public:
    virtual ~Object() {}

    /// Returns the type name for the object.
    const std::string & GetTypeName() const { return type_name_; }

    /// Returns the name assigned to the object, which may be empty.
    const std::string & GetName() const { return name_; }

    /// Handy function that returns a string describing the object, including
    /// its name (if it has one) and address.
    std::string GetDesc() const;

    /// Returns true if this Object instance was created as a template (in the
    /// TEMPLATES field of some Object).
    bool IsTemplate() const {
        return instance_type_ == InstanceType_::kTemplate;
    }

    /// Returns true if the Object was created with a CLONE statement or by
    /// direct cloning.
    bool IsClone() const { return instance_type_ == InstanceType_::kClone; }

    /// Creates a clone of this instance and casts to the templated type.
    /// Asserts if it fails.
    template <typename T>
    std::shared_ptr<T> CloneTyped(bool is_deep,
                                  const std::string &name = "") const {
        std::shared_ptr<T> clone =
            Util::CastToDerived<T>(Clone_(name, is_deep, true));
        ASSERT(clone);
        return clone;
    }

    /// When cloning an instance with is_deep set to true, all nested Object
    /// instances will be cloned. This function can be overridden in derived
    /// classes to change this behavior. It will be called for any Object that
    /// is about to be cloned because it is nested within an Object being
    /// cloned with is_deep true. The base class defines this to always return
    /// true.
    virtual bool ShouldDeepClone() const { return true; }

    /// Returns true if the Object is scoped. A Scoped object can contain
    /// Constants and Templates. Only Scoped objects are searched for
    /// Constants, Templates, and Objects for USE and CLONE statements. The
    /// base class defines this to return true.
    virtual bool IsScoped() const { return true; }

    /// Returns true if an object of this type requires a name. The default
    /// implementation returns false.
    virtual bool IsNameRequired() const { return false; }

    /// Returns the field with the given name, or a null pointer if none has
    /// that name.
    Field * FindField(const std::string &name) const;

    /// Access to all fields, for Writer mostly.
    const std::vector<Field*> & GetFields() const { return fields_; }

    /// Returns true if any field was set since construction.
    bool WasAnyFieldSet() const;

  protected:
    /// The constructor is protected to make this abstract.
    Object() {}

    /// \name Object creation functions.
    /// These virtual functions are invoked during creation of Object
    /// instances. See the class documentation for more details.
    ///@{

    /// This is called after construction to let derived classes do any
    /// post-construction work (which can use virtual functions, unlike a
    /// constructor). The base class defines this to do nothing.
    virtual void ConstructionDone() {}

    /// Derived classes may implement this function to add fields that can be
    /// parsed. The base class implements this to do nothing.
    virtual void AddFields() {}

    /// This is called when a field belonging to the object is parsed. The
    /// default implementation does nothing, but derived classes may find this
    /// information useful.
    virtual void SetFieldParsed(const Field &field) {}

    /// This is used for setting up clones: it copies the contents from the
    /// given instance into this one. The instance is guaranteed to be of the
    /// same type. This class defines it to just copy all parsed fields.
    /// Derived classes can copy additional state after calling the base
    /// class's version.
    virtual void CopyContentsFrom(const Object &from, bool is_deep);

    /// This is called to validate a non-template instance after all fields
    /// have been parsed. If there is anything wrong with the instance, this
    /// should fill details with useful error information and return false. The
    /// base class defines this to just return true.
    virtual bool IsValid(std::string &details) { return true; }

    /// This is called for each instance (including templates) after cloning is
    /// done and after all fields have been parsed (unless IsValid() returned
    /// false). This allows derived classes to perform any post-parsing or
    /// post-construction setup for new instances and clones. The IsTemplate()
    /// and IsClone() functions can be called to determine the type of instance
    /// if it matters. The base class defines this to guard against multiple
    /// calls. Derived classes should probably call their base class first
    /// before doing their own work.
    virtual void CreationDone() {
        ASSERT(instance_type_ != InstanceType_::kUnknown);
        ASSERT(! is_creation_done_);
        is_creation_done_ = true;
    }

    ///@}

    /// Returns true if CreationDone() was called for this instance.
    bool IsCreationDone() const { return is_creation_done_; }

    /// Derived classes can call this in their AddFields() function to add a
    /// field to the vector. It is assumed that the storage for the field lasts
    /// at least as long as the Object instance.
    void AddField(Field &field) { fields_.push_back(&field); }

    /// Sets the name in an instance.
    void SetName(const std::string &name) { name_ = name; }

    /// Sets the type name for the object.
    void SetTypeName(const std::string &type_name) { type_name_ = type_name; }

  private:
    /// Type of instance.
    enum class InstanceType_ {
        kUnknown,   ///< Status unknown until CreationDone() is called.
        kTemplate,  ///< This is a template Object.
        kClone,     ///< This is a clone of a template or regular Object.
        kRegular,   ///< Neither a template nor a clone.
    };

    std::string   type_name_;  ///< Name of the object's type.
    std::string   name_;       ///< Optional name assigned in file.

    /// Type of instance, set just before CreationDone() is called.
    InstanceType_ instance_type_ = InstanceType_::kUnknown;

    /// Set to true in CreationDone() to guard against multiple calls.
    bool is_creation_done_ = false;

    /// Fields added by derived classes. Note that these are raw pointers so
    /// that the Object does not take ownership.
    std::vector<Field*> fields_;

    /// Instances should never be copied, so delete the copy constructor.
    Object(const Object &obj) = delete;

    /// Instances should never be copied, so delete the assignment operator.
    Object & operator=(const Object &obj) = delete;

    /// Sets up an instance initially.
    void SetUp_(const std::string &type_name, const std::string &name,
                bool is_complete);

    /// Returns a clone of the Object (or derived class). If is_deep is true,
    /// this does a deep clone, meaning that all fields containing Objects have
    /// their Objects cloned as well. The name (if not empty) is used for the
    /// clone instead of copying the name. If is_complete is true, this also
    /// calls CreationDone().
    ObjectPtr Clone_(const std::string &name, bool is_deep,
                     bool is_complete) const;

    /// Indicates that an instance is complete, calling CreationDone().
    void CompleteInstance_(InstanceType_ instance_type);

    friend class Parser;    // Allow to define as clone.
    friend class Registry;  // Allow to create and set up instances.
};

}  // namespace Parser
