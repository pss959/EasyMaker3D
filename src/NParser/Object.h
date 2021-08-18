#pragma once

#include <memory>
#include <string>
#include <vector>

namespace NParser {

//! Object is an abstract base class for all objects resulting from parsing.
class Object {
  public:
    virtual ~Object() {}

    //! Returns the type name for the object.
    const std::string & GetTypeName() const { return type_name_; }

    //! Returns the name assigned to the object, which may be empty.
    const std::string & GetName() const { return name_; }

    //! Instances should never be copied, so delete the copy constructor.
    Object(const Object &obj) = delete;

    //! Instances should never be copied, so delete the assignment operator.
    Object & operator=(const Object &obj) = delete;

  protected:
    //! The constructor is protected to make this abstract.
    Object() {}

    //! This is invoked when the parser has finished parsing the contents of an
    //! object. The base class defines this to do nothing. Derived classes may
    //! redefine this to do any post-parsing work; they should throw a
    //! NParser::Exception if anything goes wrong.
    virtual void Finalize() {}

  private:
    std::string type_name_;  //!< Name of the object's type.
    std::string name_;       //!< Optional name assigned in file.

    //! Sets the type name for the object.
    void SetTypeName_(const std::string &type_name) { type_name_ = type_name; }

    //! Sets the name in an instance.
    void SetName_(const std::string &name) { name_ = name; }

    friend class Parser;
};

typedef std::shared_ptr<Object> ObjectPtr;

}  // namespace Parser
