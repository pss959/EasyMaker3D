#pragma once

#include <memory>
#include <string>
#include <vector>

#include "NParser/ValueType.h"
#include "NParser/Value.h"

namespace NParser {

//! Object is an abstract base class for all objects resulting from parsing.
//!
//! \ingroup NParser
class Object {
  public:
    // XXXX
    struct FieldSpec {
        //! Convenience typedef for the value-storing function.
        typedef std::function<void(const std::vector<Value> &)> StoreFunc;

        std::string name;    //!< Name of the field.
        ValueType   type;    //!< Type of value stored.
        size_t      count;   //!< Number of values.

        //! Function used to store the parsed value of the field.
        StoreFunc   store_func;

        //! Convenience constructor.
        FieldSpec(const std::string &name_in, ValueType type_in,
                  int count_in, const StoreFunc &func_in) :
            name(name_in), type(type_in), count(count_in), store_func(func_in) {
        }
    };

    virtual ~Object() {}

    //! Returns the type name for the object.
    std::string GetTypeName() const { return type_name_; }

    //! XXXX
    virtual const std::vector<FieldSpec> & GetFieldSpecs() const = 0;

  protected:
    //! The constructor is protected to make this abstract.
    Object() {}

  private:
    std::string type_name_;  //!< Name of the object's type.
    std::string name_;       //!< Optional name assigned in file.

    //! Sets the type name for the object.
    void SetTypeName_(const std::string &type_name) { type_name_ = type_name; }

    //! Sets the name in an instance.
    void SetName_(const std::string &name) { name_ = name; }

    friend class Parser;
};

// XXXX
typedef std::shared_ptr<Object> ObjectPtr;

}  // namespace Parser
