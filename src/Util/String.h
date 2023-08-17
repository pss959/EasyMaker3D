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
template <typename T> Str ToString(const T &t, size_t width = 0) {
    std::ostringstream out;
    if (width)
        out << std::left << std::setw(width);
    out << t;
    return out.str();
}

/// Special case for bool.
Str ToString(bool b, bool shorten = false);

/// Returns a string containing N spaces.
inline Str Spaces(int n) { return Str(n, ' '); }

/// Converts a string to upper-case (ASCII) as well as possible.
Str ToUpperCase(const Str &s);

/// Converts a string to lower-case (ASCII) as well as possible.
Str ToLowerCase(const Str &s);

/// Returns true if two strings are equal, ignoring case.
bool StringsEqualNoCase(const Str &s1, const Str &s2);

/// Compares two strings, returning true if they are equal, and setting index
/// to the index of the first different character if they are not equal.
bool CompareStrings(const Str &s1, const Str &s2, size_t &index);

/// Returns a string with all instances of from replaced with to.
Str ReplaceString(const Str &s, const Str &from, const Str &to);

/// Returns a string formed by joining the given vector of strings.
Str JoinStrings(const StrVec &strings, const Str &glue = " ");

/// Returns the result of calling JoinStrings() on the result of calling
/// Util::ToString() for each vector element.
template <typename T> Str JoinItems(const std::vector<T> &items,
                                    const Str &glue = " ") {
    StrVec strings;
    std::transform(items.begin(), items.end(), std::back_inserter(strings),
                   [](const T &t){ return ToString(t); });
    return JoinStrings(strings, glue);
}

/// Returns the result of calling JoinStrings() on the result of calling
/// the class's ToString() on each vector element.
template <typename T> Str JoinItems2(const std::vector<T> &items,
                                     const Str &glue = " ") {
    StrVec strings;
    std::transform(items.begin(), items.end(), std::back_inserter(strings),
                   [](const T &t){ return t.ToString(); });
    return JoinStrings(strings, glue);
}

/// Removes the first N characters from a string, returning the result. If N is
/// too large, this returns an empty string.
Str RemoveFirstN(const Str &s, size_t n);

/// Converts a camel-case string to separate words. For example, "HelloThere"
/// is converted to "Hello There". If remove_first is true, the first character
/// is ignored; this can be useful for enums (e.g., "kSomeThing" results in
/// "Some Thing").
Str SplitCamelCase(const Str &s, bool remove_first);

/// Converts string \p s to a signed base 10 integer and stores it in \p n.
/// Returns false if there is any error. Note that the entire string must
/// represent the number; no whitespace is allowed.
bool StringToInteger(const Str &s, int &n);

/// Platform-specific C++ name demangling. For use with typeid(), for example.
Str Demangle(const Str &mangled_name);

#ifdef ION_PLATFORM_WINDOWS
/// Helper for annoying wide string conversion on Windows.
inline Str FromWString(const std::wstring &w) {
    return Str(w.begin(), w.end());
}

/// Helper for annoying wide string conversion on Windows.
inline std::wstring ToWString(const Str &s) {
    return std::wstring(s.begin(), s.end());
}
#endif

}  // namespace Util
