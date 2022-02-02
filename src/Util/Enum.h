#pragma once

#include <ostream>
#include <vector>

#include <magic_enum.hpp>

#include "Util/String.h"

namespace Util {

/// \name Enumerated Type Helpers
///@{

template <typename EnumType>
constexpr size_t EnumCount() {
    return magic_enum::enum_count<EnumType>();
}

/// Converts an enum value to a string.
template <typename EnumType>
std::string EnumName(EnumType e) {
    return std::string(magic_enum::enum_name<EnumType>(e));
}

/// Converts an enum value to an int.
template <typename EnumType>
int EnumInt(EnumType e) {
    return static_cast<int>(magic_enum::enum_integer<EnumType>(e));
}

/// Returns a vector of all values for the given enum type.
template <typename EnumType>
std::vector<EnumType> EnumValues() {
    constexpr auto values = magic_enum::enum_values<EnumType>();
    return std::vector<EnumType>(values.begin(), values.end());
}

/// Accesses an enum value from a string. Returns false if the string does
/// represent a valid value.
template <typename EnumType>
bool EnumFromString(const std::string &s, EnumType &result) {
    auto val = magic_enum::enum_cast<EnumType>(s);
    if (val.has_value()) {
        result = val.value();
        return true;
    }
    return false;
}

/// Convenience that converts an enum value to a string and then removes the
/// leading "k".
template <typename EnumType>
std::string EnumToWord(EnumType e) {
    return RemoveFirstN(EnumName(e), 1);
}

/// Convenience that converts an enum value to a string and then splits it from
/// camel case to separate words, removing the leading "k".
template <typename EnumType>
std::string EnumToWords(EnumType e) {
    return SplitCamelCase(EnumName(e), true);
}

///@}

}  // namespace Util

// Provide ostream operators for enums except in unit tests, where gtest has
// issues with namespace lookup.
#if ! defined(IN_UNIT_TEST)
using namespace magic_enum::ostream_operators;
#endif
