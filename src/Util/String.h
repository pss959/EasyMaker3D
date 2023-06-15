#pragma once

#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

/// \file
/// This file contains general functions for dealing with character strings.
///
/// \ingroup Utility

namespace Util {

/// Converts any printable type to a string. If the width is not zero, this
/// right pads with spaces to match the width.
template <typename T> std::string ToString(const T &t, size_t width = 0) {
    std::ostringstream out;
    if (width)
        out << std::left << std::setw(width);
    out << t;
    return out.str();
}

/// Special case for bool.
std::string ToString(bool b, bool shorten = false);

/// Returns a string containing N spaces.
inline std::string Spaces(int n) { return std::string(n, ' '); }

/// Converts a string to upper-case (ASCII) as well as possible.
std::string ToUpperCase(const std::string &s);

/// Converts a string to lower-case (ASCII) as well as possible.
std::string ToLowerCase(const std::string &s);

/// Checks for case-sensitive string containment.
bool StringContains(const std::string &container, const std::string &containee);

/// Returns true if two strings are equal, ignoring case.
bool StringsEqualNoCase(const std::string &s1, const std::string &s2);

/// Compares two strings, returning true if they are equal, and setting index
/// to the index of the first different character if they are not equal.
bool CompareStrings(const std::string &s1, const std::string &s2,
                    size_t &index);

/// Returns a string with all instances of from replaced with to.
std::string ReplaceString(const std::string &s,
                          const std::string &from,
                          const std::string &to);

/// Returns a string formed by joining the given vector of strings.
std::string JoinStrings(const std::vector<std::string> &strings,
                        const std::string &glue = " ");

/// Returns the result of calling JoinStrings() on the result of calling
/// Util::ToString() for each vector element.
template <typename T> std::string JoinItems(const std::vector<T> &items,
                                            const std::string &glue = " ") {
    std::vector<std::string> strings;
    std::transform(items.begin(), items.end(), std::back_inserter(strings),
                   [](const T &t){ return ToString(t); });
    return JoinStrings(strings, glue);
}

/// Returns the result of calling JoinStrings() on the result of calling
/// the class's ToString() on each vector element.
template <typename T> std::string JoinItems2(const std::vector<T> &items,
                                             const std::string &glue = " ") {
    std::vector<std::string> strings;
    std::transform(items.begin(), items.end(), std::back_inserter(strings),
                   [](const T &t){ return t.ToString(); });
    return JoinStrings(strings, glue);
}

/// Removes the first N characters from a string, returning the result. If N is
/// too large, this returns an empty string.
std::string RemoveFirstN(const std::string &s, size_t n);

/// Converts a camel-case string to separate words. For example, "HelloThere"
/// is converted to "Hello There". If remove_first is true, the first character
/// is ignored; this can be useful for enums (e.g., "kSomeThing" results in
/// "Some Thing").
std::string SplitCamelCase(const std::string &s, bool remove_first);

/// Converts string \p s to a signed base 10 integer and stores it in \p n.
/// Returns false if there is any error. Note that the entire string must
/// represent the number; no whitespace is allowed.
bool StringToInteger(const std::string &s, int &n);

/// Platform-specific C++ name demangling. For use with typeid(), for example.
std::string Demangle(const std::string &mangled_name);

#ifdef ION_PLATFORM_WINDOWS
/// Helper for annoying wide string conversion on Windows.
inline std::string FromWString(const std::wstring &w) {
    return std::string(w.begin(), w.end());
}

/// Helper for annoying wide string conversion on Windows.
inline std::wstring ToWString(const std::string &s) {
    return std::wstring(s.begin(), s.end());
}
#endif

}  // namespace Util
