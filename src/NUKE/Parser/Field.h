#pragma once

#include <vector>

#include "Parser/FieldSpec.h"

namespace Parser {

//! This is the base struct for a parsed Field. It stores a reference to the
//! FieldSpec used to parse the field. The value or values are stored in
//! derived versions of this struct.
//!
//! \ingroup Parser
struct Field {
    const FieldSpec &spec;   //!< FieldSpec for the field.

    //! The constructor is passed the FieldSpec for the field.
    Field(const FieldSpec &spec_in) : spec(spec_in) {}

    virtual ~Field() {}

    //! Returns a value of the templated type. Throws an Exception if the
    //! field type is not consistent with that type or if this is actually
    //! an ArrayField.
    template <typename T> const T & GetValue() const;

    //! Returns a vector of values of the templated type. The number of values
    //! is defined by the count in the spec. Throws an Exception if the field
    //! type is not consistent with the templated type or if the count in the
    //! field is 1.
    template <typename T> std::vector<T> GetValues() const;
};

}  // namespace Parser
