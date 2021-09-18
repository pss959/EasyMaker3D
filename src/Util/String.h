#pragma once

#include <string>
#include <sstream>

namespace Util {

//! \name String Utilities
//!@{

//! Converts any printable type to a string.
template <typename T>
std::string ToString(const T &t) {
    std::ostringstream out;
    out << t;
    return out.str();
}

//! Returns a string containing N spaces.
inline std::string Spaces(int n) { return std::string(n, ' '); }

//! Returns true if two strings are equal, ignoring case.
bool StringsEqualNoCase(const std::string &s1, const std::string &s2);

//! Compares two strings, returning true if they are equal, and setting index
//! to the index of the first different character if they are not equal.
bool CompareStrings(const std::string &s1, const std::string &s2,
                    size_t &index);

// Returns a string with all instances of from replaced with to.
std::string ReplaceString(const std::string &s,
                          const std::string &from,
                          const std::string &to);

//! Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

//!@}


}  // namespace Util
