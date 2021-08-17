#pragma once

#include <string>
#include <vector>

#include "NParser/ValueType.h"
#include "NParser/Value.h"

namespace NParser {

class Object;

//! XXXX
//!
//! \ingroup NParser
struct FieldSpec {
    //! Convenience typedef for a value-storing function.
    typedef std::function<void(Object &obj,
                               const std::vector<Value> &)> StoreFunc;

    std::string name;        //!< Name of the field.
    ValueType   type;        //!< Type of value stored.
    size_t      count;       //!< Number of values.
    StoreFunc   store_func;  //!< Stores the parsed value somewhere.

    //! Convenience constructor
    FieldSpec(const std::string &name_in, ValueType type_in, size_t count_in,
              const StoreFunc &func_in) :
        name(name_in), type(type_in), count(count_in), store_func(func_in) {}
};

}  // namespace NParser
