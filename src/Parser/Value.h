#pragma once

#include <variant>
#include <vector>

#include "Parser/Typedefs.h"

namespace Parser {

//! Internal typedef for an std::variant that can hold a value of any type
//! supported by the Parser.
typedef std::variant<bool,
                     int,
                     float,
                     std::string,
                     ObjectPtr,
                     std::vector<ObjectPtr>> Value;

}  // namespace Parser
