#pragma once

#include <string>

namespace Graph {

class Reader_;

//! Object is an abstract base class for all Graph classes. It supports naming.
class Object {
    //! Returns the name, which will be empty if never set.
    const std::string & GetName() const { return name_; }

  protected:
    Object() {}
    virtual ~Object() {}

    //! Sets the name for an object. Derived classes can add side effects, such
    //! as setting labels in Ion objects.
    virtual void SetName_(const std::string &name) { name_ = name; }

  private:
    std::string name_;

    friend class Reader_;
};

}  // namespace Graph
