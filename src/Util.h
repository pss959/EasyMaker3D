#pragma once

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include <algorithm>
#include <memory>
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
