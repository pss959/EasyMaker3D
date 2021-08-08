#pragma once

#include <string>
#include <vector>

#include "Parser/Typedefs.h"

namespace Parser {

//! The Object struct represents a parsed object.
struct Object {
    std::string           type_name;    //!< Name of the object's type.
    std::vector<FieldPtr> fields;       //!< Fields parsed for the object.
    std::string           path;         //!< Path read from.
    int                   line_number;  //!< Line number of definition.
};

}  // namespace Parser
