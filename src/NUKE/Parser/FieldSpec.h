#pragma once

#include <string>

#include "Parser/ValueType.h"

namespace Parser {

//! Specification for a field to parse. A vector of FieldSpec instances is
//! provided to the Parser so it knows how to parse fields in objects.
//!
//! \ingroup Parser
struct FieldSpec {
    std::string name;       //!< Name of the field.
    ValueType   type;       //!< Base type of the field's value.
    uint32_t    count = 1;  //!< Fixed number of values to parse. Defaults to 1.

    //! Equality test.
    bool operator==(const FieldSpec &other) const {
        return name == other.name && type == other.type &&
            count == other.count;
    }

    //! Inequality test.
    bool operator!=(const FieldSpec &other) const {
        return ! (*this == other);
    }
};

}  // namespace Parser
