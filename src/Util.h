#pragma once

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include <string>

//! This namespace contains general utility functions and classes.
namespace Util {

//! Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

}  // namespace Util
