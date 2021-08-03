#pragma once

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include <memory>
#include <string>

//! This namespace contains general utility functions and classes.
namespace Util {

//! Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

//! Convenience to cast an std::shared_ptr of a derived class to an
//! std::shared_ptr of a base class.
template <typename Base, typename Derived>
std::shared_ptr<Base> CastToBase(const std::shared_ptr<Derived> &ptr) {
    return std::shared_ptr<Base>(ptr.get());
}

}  // namespace Util
