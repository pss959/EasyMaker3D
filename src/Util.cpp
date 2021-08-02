#include "Util.h"

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include <string>

namespace Util {

std::string Demangle(const std::string &mangled_name) {
#ifdef __GNUG__
    int status = 0 ;
    char *demangled = __cxxabiv1::__cxa_demangle(
        mangled_name.c_str(), nullptr, nullptr, &status);
    if (! demangled)
        return mangled_name;
    std::string ret(demangled);
    free(demangled);
    return ret;
#else
    return mangled_name;
#endif
}

}  // namespace Util
