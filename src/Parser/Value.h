#pragma once

#include <memory>
#include <variant>
#include <vector>

namespace Parser {

class Object;

//! Internal typedef for an std::variant that can hold a value of any type
//! supported by the Parser.
typedef std::variant<bool,
                     int,
                     unsigned int,
                     float,
                     std::string,
                     std::shared_ptr<Object>,
                     std::vector<std::shared_ptr<Object>>> Value;

}  // namespace Parser
