#pragma once

#include <functional>
#include <string>
#include <vector>

#include "NParser/FieldSpec.h"
#include "NParser/ValueType.h"
#include "NParser/Value.h"

namespace NParser {

class Object;

//! A ObjectSpec provides information about a derived Parser::Object class that
//! can be created during parsing.
struct ObjectSpec {
    //! Convenience typedef for a function used to create an instance of the
    //! Object.
    typedef std::function<Object *()> CreationFunc;

    //! Type name for the object as it appears in parsed data.
    std::string            type_name;

    //! Flag indicating whether a name is required for an instance.
    bool                   is_name_required;

    //! Function that is invoked to create a new instance.
    CreationFunc           creation_func;

    //! Vector of FieldSpec instances that specify what fields can be parsed
    //! for an instance and how to store the parsed values.
    std::vector<FieldSpec> field_specs;
};

}  // namespace NParser
