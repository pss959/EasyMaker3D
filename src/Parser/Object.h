#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Parser/Field.h"
#include "Util/Assert.h"
#include "Util/General.h"

namespace Parser {

class Object;
typedef std::shared_ptr<Object> ObjectPtr;

/// Parser::Object is an abstract base class for all objects resulting from
/// parsing. All classes derived from this typically contain Parser::Field
/// instances that allow class-specific data to be read and stored. Derived
/// classes must add these fields after construction so the Parser can access
/// them; the virtual AddFields() function is used to do this.
///
/// Because of this setup, the following requirements are placed on all derived
/// Object classes:
///   - If the class has a constructor, it must be protected.
///   - If the class is concrete, it must be registered with the
///     Parser::Registry so that instances may be created by the Parser.
///   - The class should add Parser::Registry as a friend so that the registry
///     can create instances of it.
///   - Creating an instance of any derived concrete class must be done via
///     Parser::Registry::CreateObjectOfType() so that the instance is set up
///     correctly.
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

    /// Returns true if the Object was created with a CLONE statement or by
    /// direct cloning.
    bool IsClone() const { return is_clone_; }

    /// Returns true if the Object is scoped. A Scoped object can contain
    /// Constants and Templates. Only Scoped objects are searched for
    /// Constants, Templates, and Objects for USE and CLONE statements. The
    /// base class defines this to return true.
    virtual bool IsScoped() const { return true; }

    /// Returns true if an object of this type requires a name. The default
    /// implementation returns false.
    virtual bool IsNameRequired() const { return false; }

    /// Derived classes may implement this function to add fields that can be
    /// parsed. The base class implements this to do nothing.
    virtual void AddFields() {}

    /// This is called when a field belonging to the object is parsed. The
    /// default implementation does nothing, but derived classes may find this
    /// information useful.
    virtual void SetFieldParsed(const Field &field) {}

    /// This is called when all fields belonging to the object have been
    /// parsed. If there is anything wrong with the derived class's instance,
    /// this should fill details with useful error information and return
    /// false. The base class defines this to just return true.
    virtual bool IsValid(std::string &details) { return true; }

    /// Returns the field with the given name, or a null pointer if none has
    /// that name.
    Field * FindField(const std::string &name) const;

    /// Access to all fields, for Writer mostly.
    const std::vector<Field*> & GetFields() const { return fields_; }

    /// Returns a clone of the Object (or derived class). If is_deep is true,
    /// this does a deep clone, meaning that all fields containing Objects have
    /// their Objects cloned as well. The optional name is used for the clone.
    ///
    /// Note that this just creates the clone and then calls CopyContentsFrom()
    /// on it. Derived classes can just redefine that function to copy any
    /// extra state.
    ObjectPtr Clone(bool is_deep, const std::string &name = "") const;

    /// Convenience that clones the Object and casts to the templated type.
    /// Asserts if this fails.
    template <typename T>
    std::shared_ptr<T> CloneTyped(bool is_deep,
                                  const std::string &name = "") const {
        std::shared_ptr<T> clone = Util::CastToDerived<T>(Clone(is_deep, name));
        ASSERT(clone);
        return clone;
    }

    /// When doing a Clone() operation with is_deep set to true, all nested
    /// Object instances will be cloned. This function can be overridden in
    /// derived classes to change this behavior. It will be called for any
    /// Object that is about to be cloned because it is nested within an Object
    /// being cloned with is_deep true. The base class defines this to always
    /// return true.
    virtual bool ShouldDeepClone() const { return true; }

  protected:
    /// The constructor is protected to make this abstract.
    Object() {}

    /// This is called after construction to let derived classes do any
    /// post-construction work (which can use virtual functions, unlike a
    /// constructor). The base class defines this to do nothing.
    virtual void ConstructionDone() {}

    /// Derived classes can call this in their AddFields() function to add a
    /// field to the vector. It is assumed that the storage for the field lasts
    /// at least as long as the Object instance.
    void AddField(Field &field) { fields_.push_back(&field); }

    /// Sets the type name for the object.
    void SetTypeName(const std::string &type_name) { type_name_ = type_name; }

    /// Sets the name in an instance.
    void SetName(const std::string &name) { name_ = name; }

    /// This is used for setting up clones: it copies the contents from the
    /// given instance into this one. The instance is guaranteed to be of the
    /// same type. This class defines it to just copy all parsed
    /// fields. Derived classes can copy additional state after calling the
    /// base class's version.
    virtual void CopyContentsFrom(const Object &from, bool is_deep);

  private:
    std::string type_name_;  ///< Name of the object's type.
    std::string name_;       ///< Optional name assigned in file.
    bool is_clone_ = false;  ///< True if this was created as a clone.

    /// Fields added by derived classes. Note that these are raw pointers so
    /// that the Object does not take ownership.
    std::vector<Field*> fields_;

    /// Instances should never be copied, so delete the copy constructor.
    Object(const Object &obj) = delete;

    /// Instances should never be copied, so delete the assignment operator.
    Object & operator=(const Object &obj) = delete;

    /// Lets the Parser define this as a clone.
    void SetIsClone() { is_clone_ = true; }

    friend class Parser;
    friend class Registry;
};

}  // namespace Parser
