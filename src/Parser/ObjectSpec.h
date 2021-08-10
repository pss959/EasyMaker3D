#pragma once

#include <string>

#include "Parser/FieldSpec.h"

namespace Parser {

//! Specification for a object to parse. A vector of ObjectSpec instances is
//! provided to the Parser so it knows how to parse objects.
//! \ingroup Parser
struct ObjectSpec {
    //! Name of the object's type.
    std::string type_name;

    //! FieldSpec instances for all fields that can be found in an object of
    //! this type.
    std::vector<FieldSpec> field_specs;
};

}  // namespace Parser
