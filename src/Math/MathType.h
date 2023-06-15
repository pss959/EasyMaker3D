#pragma once

#include <ostream>
#include <type_traits>

/// MathType is an empty struct that serves as a base for all Math types that
/// support a ToString() function to help with debugging. It defines the
/// insertion operator for convenience.
///
/// \ingroup Math
struct MathType {
    bool operator==(const MathType &t) const = default;
};

/// Output operator for anything derived from MathType.
template <typename T>
inline typename std::enable_if<std::is_base_of<MathType, T>::value,
                               std::ostream>::type &
operator<<(std::ostream& out, const T &t) {
    return out << t.ToString();
}
