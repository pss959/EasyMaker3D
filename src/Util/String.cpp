#include "Util/String.h"

#include <algorithm>

#include <ion/base/stringutils.h>

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include "Math/Linear.h"

namespace Util {

std::string ToString(bool b, bool shorten) {
    std::ostringstream out;
    if (shorten)
        out << (b ? "T" : "F");
    else
        out << (b ? "True" : "False");
    return out.str();
}

std::string ToString(float f, float precision) {
    return ToString(RoundToPrecision(f, precision));
}

std::string ToString(const ion::math::VectorBase<2, float> &v, float precision) {
    return "[" +
        ToString(v[0], precision) + ", " +
        ToString(v[1], precision) + "]";
}

std::string ToString(const ion::math::VectorBase<3, float> &v, float precision) {
    return "[" +
        ToString(v[0], precision) + ", " +
        ToString(v[1], precision) + ", " +
        ToString(v[2], precision) + "]";
}

bool StringContains(const std::string &container,
                    const std::string &containee) {
    return container.find(containee) != std::string::npos;
}

bool StringsEqualNoCase(const std::string &s1, const std::string &s2) {
    auto compare_chars = [](const char &c1, const char &c2){
        return c1 == c2 || std::toupper(c1) == std::toupper(c2);
    };
    return s1.size() == s2.size() &&
        std::equal(s1.begin(), s1.end(), s2.begin(), compare_chars);
}

bool CompareStrings(const std::string &s1, const std::string &s2,
                    size_t &index) {
    if (s1 == s2)
        return true;
    for (size_t i = 0; i < s1.size(); ++i) {
        if (i >= s2.size() || s1[i] != s2[i]) {
            index = i;
            return false;
        }
    }
    // If it gets here, s2 must be longer than s1.
    index = s1.size();
    return false;
}

std::string ReplaceString(const std::string &s,
                          const std::string &from,
                          const std::string &to) {
    return ion::base::ReplaceString(s, from, to);
}

std::string JoinStrings(const std::vector<std::string> &strings,
                        const std::string &glue) {
    return ion::base::JoinStrings(strings, glue);
}

std::string RemoveFirstN(const std::string &s, size_t n) {
    const size_t nn = std::min(n, s.length());
    return s.substr(nn, s.length() - nn);
}

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
