#include "Util/String.h"

#include <cxxabi.h>  // For demangling.

#include <algorithm>
#include <cctype>

#include <ion/base/stringutils.h>

namespace Util {

std::string ToString(bool b, bool shorten) {
    std::ostringstream out;
    if (shorten)
        out << (b ? "T" : "F");
    else
        out << (b ? "True" : "False");
    return out.str();
}

std::string ToUpperCase(const std::string &s) {
    std::string u;
    std::transform(s.begin(), s.end(), std::back_inserter(u),
                   [](unsigned char c){ return std::toupper(c); });
    return u;
}

std::string ToLowerCase(const std::string &s) {
    std::string u;
    std::transform(s.begin(), s.end(), std::back_inserter(u),
                   [](unsigned char c){ return std::tolower(c); });
    return u;
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

std::string SplitCamelCase(const std::string &s, bool remove_first) {
    std::string ss;

    // A string of 2 or more upper-case letters followed by a lower-case letter
    // also represents a split. For example, "CSGUnion" -> "CSG Union".

    const size_t count = s.size();
    bool was_lower = false;
    int  upper_count = 0;
    for (size_t i = remove_first ? 1 : 0; i < count; ++i) {
        const char c = s[i];
        if (isupper(c)) {
            if (was_lower)
                ss += ' ';
            was_lower = false;
            ++upper_count;
        }
        else {
            if (upper_count > 1) {  // Case described above.
                const char l = ss.back();
                ss.back() = ' ';
                ss += l;
            }
            was_lower = true;
            upper_count = 0;
        }
        ss += c;
    }
    return ss;
}

bool StringToInteger(const std::string &s, int &n) {
    // Integers may be signed and are always base 10.
    if (! s.empty() && ! std::isspace(s[0])) {
        try {
            size_t chars_processed;
            n = std::stoi(s, &chars_processed, 10);
            if (chars_processed == s.size())  // No extra characters at the end.
                return true;
        }
        catch (std::exception &) {} // Fall through to return below.
    }
    return false;
}

std::string Demangle(const std::string &mangled_name) {
    int status = 0 ;
    char *demangled = __cxxabiv1::__cxa_demangle(
        mangled_name.c_str(), nullptr, nullptr, &status);
    if (! demangled)
        return mangled_name;
    std::string ret(demangled);
    free(demangled);
    return ret;
}

}  // namespace Util
