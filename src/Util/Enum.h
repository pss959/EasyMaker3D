#pragma once

#include <ostream>
#include <vector>

// magic_enum has a maximum value of 128 for enum values. This is normally not
// a problem, but for bit-shifted flags it can be. This sets the minimum and
// maximum so that more flags can be used.
#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 1024
#include <magic_enum.hpp>

#include "Util/String.h"

/// \file
/// This file contains functions that help deal with converting between
/// enumerated types and values, names, etc. They all use the terrific
/// magic_enum package.
///
/// \ingroup Utility

namespace Util {

template <typename EnumType>
constexpr size_t EnumCount() {
    return magic_enum::enum_count<EnumType>();
}

/// Converts an enum value to a string.
template <typename EnumType>
Str EnumName(EnumType e) {
    return Str(magic_enum::enum_name<EnumType>(e));
}

/// Converts an enum value to an int.
template <typename EnumType>
int EnumInt(EnumType e) {
    return static_cast<int>(magic_enum::enum_integer<EnumType>(e));
}

/// Converts an enum value from an index into the enums (not the actual value).
/// Note that this asserts on error.
template <typename EnumType>
EnumType EnumFromIndex(int value) {
    return magic_enum::enum_value<EnumType>(value);
}

/// Returns an array of all values for the given enum type.
template <typename EnumType>
constexpr auto EnumValuesArray() {
    return magic_enum::enum_values<EnumType>();
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
bool EnumFromString(const Str &s, EnumType &result) {
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
Str EnumToWord(EnumType e) {
    return RemoveFirstN(EnumName(e), 1);
}

/// Convenience that converts an enum value to a string and then splits it from
/// camel case to separate words, removing the leading "k".
template <typename EnumType>
Str EnumToWords(EnumType e) {
    return SplitCamelCase(EnumName(e), true);
}

}  // namespace Util

// Provide ostream operators for enums except in unit tests, where gtest has
// issues with namespace lookup.
#if ! defined(IN_UNIT_TEST)
using namespace magic_enum::ostream_operators;
#endif
