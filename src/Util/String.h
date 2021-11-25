#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

#include "Math/Types.h"

namespace Util {

/// \name String Utilities
///@{

/// Converts any printable type to a string.
template <typename T>
std::string ToString(const T &t) {
    std::ostringstream out;
    out << t;
    return out.str();
}

/// Special case for bool.
std::string ToString(bool b, bool shorten = false);

/// Special version of ToString() for floats that takes a precision.
std::string ToString(float f, float precision);

/// Special version of ToString() for vectors that takes a precision.
std::string ToString(const ion::math::VectorBase<2, float> &v, float precision);

/// Special version of ToString() for vectors that takes a precision.
std::string ToString(const ion::math::VectorBase<3, float> &v, float precision);

/// Returns a string containing N spaces.
inline std::string Spaces(int n) { return std::string(n, ' '); }

/// Checks for case-sensitive string containment.
bool StringContains(const std::string &container, const std::string &containee);

/// Returns true if two strings are equal, ignoring case.
bool StringsEqualNoCase(const std::string &s1, const std::string &s2);

/// Compares two strings, returning true if they are equal, and setting index
/// to the index of the first different character if they are not equal.
bool CompareStrings(const std::string &s1, const std::string &s2,
                    size_t &index);

// Returns a string with all instances of from replaced with to.
std::string ReplaceString(const std::string &s,
                          const std::string &from,
                          const std::string &to);

// Returns a string formed by joining the given vector of strings.
std::string JoinStrings(const std::vector<std::string> &strings,
                        const std::string &glue = " ");

// Returns the result of calling JoinStrings() on the result of calling
// Util::ToString() for each vector element.
template <typename T> std::string JoinItems(const std::vector<T> &items,
                                            const std::string &glue = " ") {
    std::vector<std::string> strings;
    std::transform(items.begin(), items.end(), std::back_inserter(strings),
                   [](const T &t){ return ToString(t); });
    return JoinStrings(strings, glue);
}

// Returns the result of calling JoinStrings() on the result of calling
// the class's ToString() on each vector element.
template <typename T> std::string JoinItems2(const std::vector<T> &items,
                                             const std::string &glue = " ") {
    std::vector<std::string> strings;
    std::transform(items.begin(), items.end(), std::back_inserter(strings),
                   [](const T &t){ return t.ToString(); });
    return JoinStrings(strings, glue);
}

/// Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

///@}


}  // namespace Util
