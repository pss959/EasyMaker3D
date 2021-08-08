#pragma once

#include <assert.h>

#include <vector>

#include "Parser/Field.h"
#include "Parser/Value.h"

namespace Parser {

//! A derived Field struct that holds multiple values in an std::vector. This
//! is used internally to the Parser class.
struct ArrayField_ : public Field {
    std::vector<Value> values;

    //! The constructor is passed the FieldSpec for the field and the vector of
    //! values to store.
    ArrayField_(const FieldSpec &spec_in, const std::vector<Value> &values_in) :
        Field(spec_in), values(values_in) {
        assert(spec_in.count > 1);
    }
};


}  // namespace Parser
