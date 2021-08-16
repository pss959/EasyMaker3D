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
struct FieldSpecs {
  public:
    //! Convenience typedef for a value-storing function.
    typedef std::function<void(Object &obj,
                               const std::vector<Value> &)> StoreFunc;

    // XXXX
    struct Spec {
        std::string name;        //!< Name of the field.
        ValueType   type;        //!< Type of value stored.
        size_t      count;       //!< Number of values.
        StoreFunc   store_func;  //!< Stores the parsed value somewhere.
    };

    std::vector<Spec> specs;

    FieldSpecs() {}

    //! Constructor that takes an initializer list of Spec instances; useful in
    //! static initializers.
    FieldSpecs(const std::initializer_list<Spec> &specs_in) : specs(specs_in) {}

    //! Concatenates two instances.
    friend FieldSpecs operator+(const FieldSpecs &s0, const FieldSpecs &s1) {
        FieldSpecs sc;
        sc.specs.insert(sc.specs.end(), s0.specs.begin(), s0.specs.end());
        sc.specs.insert(sc.specs.end(), s1.specs.begin(), s1.specs.end());
        return sc;
    }
};

}  // namespace NParser
