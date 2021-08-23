#pragma once

#include <magic_enum.hpp>

namespace Util {

//! \name Enumerated Type Helpers
//!@{

//! Converts an enum value to a string.
template <typename EnumType>
std::string EnumName(EnumType e) {
    return std::string(magic_enum::enum_name<EnumType>(e));
}

//! Converts an enum value to an int.
template <typename EnumType>
int EnumInt(EnumType e) {
    return static_cast<int>(magic_enum::enum_integer<EnumType>(e));
}

//! Accesses an enum value from a string. Returns false if the string does
//! represent a valid value.
template <typename EnumType>
bool EnumFromString(const std::string &s, EnumType &result) {
    auto val = magic_enum::enum_cast<EnumType>(s);
    if (val.has_value()) {
        result = val.value();
        return true;
    }
    return false;
}

//!@}

}  // namespace Util
