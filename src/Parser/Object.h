#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Parser/Field.h"

namespace Parser {

//! Object is an abstract base class for all objects resulting from parsing.
class Object {
  public:
    virtual ~Object() {}

    //! Returns the type name for the object.
    const std::string & GetTypeName() const { return type_name_; }

    //! Returns the name assigned to the object, which may be empty.
    const std::string & GetName() const { return name_; }

    //! Returns true if an object of this type requires a name. The default
    //! implementation returns false.
    virtual bool IsNameRequired() const { return false; }

    //! Derived classes may implement this function to add fields that can be
    //! parsed. The base class implements this to do nothing.
    virtual void AddFields() {}

    //! This is called when a field belonging to the object is parsed. The
    //! default implementation does nothing, but derived classes may find this
    //! information useful.
    virtual void SetFieldParsed(const Field &field) {}

    //! Returns the field with the given name, or a null pointer if none has
    //! that name.
    Field * FindField(const std::string &name) const {
        for (auto &field: fields_)
            if (field->GetName() == name)
                return field;
        return nullptr;
    }

    //! Access to all fields, for Writer mostly.
    const std::vector<Field*> & GetFields() const { return fields_; }

  protected:
    //! The constructor is protected to make this abstract.
    Object() {}

    //! Derived classes can call this in their AddFields() function to add a
    //! field to the vector. It is assumed that the storage for the field lasts
    //! at least as long as the Object instance.
    void AddField(Field &field) {
        fields_.push_back(&field);
    }

    //! Sets the name in an instance.
    void SetName(const std::string &name) { name_ = name; }

  private:
    std::string type_name_;  //!< Name of the object's type.
    std::string name_;       //!< Optional name assigned in file.

    //! Fields added by derived classes. Note that these are raw pointers so
    //! that the Object does not take ownership.
    std::vector<Field*> fields_;

    //! Sets the type name for the object.
    void SetTypeName_(const std::string &type_name) { type_name_ = type_name; }

    //! Instances should never be copied, so delete the copy constructor.
    Object(const Object &obj) = delete;

    //! Instances should never be copied, so delete the assignment operator.
    Object & operator=(const Object &obj) = delete;

    friend class Parser;
};

typedef std::shared_ptr<Object> ObjectPtr;

}  // namespace Parser
