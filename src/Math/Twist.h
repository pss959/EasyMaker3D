#pragma once

#include <string>

#include "Math/Types.h"

/// A Twist defines how to twist a mesh around an axis through some center
/// point.
///
/// \ingroup Math
struct Twist {
    Point3f  center{0, 0, 0};  ///< Twist center; defaults to origin.
    Vector3f axis{0, 1, 0};    ///< Twist axis; defaults to +Y axis.
    Anglef   angle;            ///< Twist angle; defaults to 0.

    /// Equality operator.
    bool operator==(const Twist &t) const {
        return t.center == center && t.axis == axis && t.angle == angle;
    }

    /// Inequality operator.
    bool operator!=(const Twist &b) const { return ! (b == *this); }

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};
