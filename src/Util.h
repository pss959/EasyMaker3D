#pragma once

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include <stdio.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>

#include <magic_enum.hpp>

#include "Flags.h"

//! This namespace contains general utility functions and classes.
namespace Util {

// ----------------------------------------------------------------------------
// General utilities.
// ----------------------------------------------------------------------------

//! \name General Utilities
//!@{

//! This class can be used to temporarily disable \c stdout and \c stderr
//! output. Any code inside its scope will be redirected to /dev/null.
class OutputMuter {
  public:
    OutputMuter() {
        saved_stdout_ = *stdout;
        saved_stderr_ = *stderr;
        *stdout = *fopen("/dev/null", "w");
        *stderr = *fopen("/dev/null", "w");
    }
    ~OutputMuter() {
        *stdout = saved_stdout_;
        *stderr = saved_stderr_;
    }
  private:
    FILE saved_stdout_;
    FILE saved_stderr_;
};

//! Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

//! Convenience to cast an std::shared_ptr of a derived class to an
//! std::shared_ptr of a base class.
template <typename Base, typename Derived>
std::shared_ptr<Base> CastToBase(const std::shared_ptr<Derived> &ptr) {
    return std::shared_ptr<Base>(ptr.get());
}

//! Returns true if an STL container contains the given element.
template <typename Container, typename Element>
bool Contains(const Container &ctr, const Element &elt) {
    return std::find(ctr.begin(), ctr.end(), elt) != ctr.end();
}

//! Converts any printable type to a string.
template <typename T>
std::string ToString(const T &t) {
    std::ostringstream out;
    out << t;
    return out.str();
}

//!@}

// ----------------------------------------------------------------------------
// Enum helpers.
// ----------------------------------------------------------------------------

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

//! Converts a Flags instance to a string of the form \c "kX|kY|...".
template <typename EnumType>
std::string EnumFlagNames(Flags<EnumType> flags) {
    std::string s = "";
    constexpr auto entries = magic_enum::enum_entries<EnumType>();
    for (auto &entry: entries) {
        if (flags.Has(entry.first)) {
            if (! s.empty())
                s += '|';
            s += std::string(entry.second);
        }
    }
    return s;
}

//!@}

}  // namespace Util
