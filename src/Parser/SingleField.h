#pragma once

#include <assert.h>

#include "Parser/Field.h"
#include "Parser/Value.h"

namespace Parser {

//! A derived Field struct that holds a single value. This is used internally
//! to the Parser class.
struct SingleField_ : public Field {
    Value value;   //!< Value storage.

    //! The constructor is passed the FieldSpec for the field and the value to
    //! store.
    SingleField_(const FieldSpec &spec_in, const Value &value_in) :
        Field(spec_in), value(value_in) {
        assert(spec_in.count == 1);
    }
};

}  // namespace Parser
