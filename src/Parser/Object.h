#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Parser/ObjectSpec.h"
#include "Parser/Typedefs.h"

namespace Parser {

//! The Object struct represents a parsed object.
//! \ingroup Parser
struct Object {
    const ObjectSpec      &spec;        //!< ObjectSpec for the object.
    const std::string     path;         //!< Path read from.
    const int             line_number;  //!< Line number of definition.

    std::string           name;         //!< Optional name assigned in file.
    std::vector<FieldPtr> fields;       //!< Fields parsed for the object.

    //! This stores paths to all files that were included within the definition
    //! of the object. This allows the caller to examine and process file
    //! dependencies.
    std::vector<std::string> included_paths;

    //! Constant definitions in the object, stored as a map from name to string
    //! value.
    std::unordered_map<std::string, std::string> constants_map;

    //! The constructor is passed the ObjectSpec for the object, the file path
    //! it came from and the line number it was defined on..
    Object(const ObjectSpec &spec_in, const std::string &path_in,
           int line_number_in) :
        spec(spec_in), path(path_in), line_number(line_number_in) {}
};

}  // namespace Parser
