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

//! Returns true if two strings are equal, ignoring case.
bool StringsEqualNoCase(const std::string &s1, const std::string &s2);

//! Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

//!@}


}  // namespace Util
