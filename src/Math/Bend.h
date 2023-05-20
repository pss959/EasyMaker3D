#pragma once

#include <string>

#include "Math/Types.h"

/// A Bend defines how to bend a mesh around XXXXX
///
/// \ingroup Math
struct Bend {
    Point3f  center{0, 0, 0};  ///< Bend center; defaults to origin.
    Vector3f axis{0, 1, 0};    ///< Bend axis; defaults to +Y axis.
    Anglef   angle;            ///< Bend angle; defaults to 0.

    /// Equality operator.
    bool operator==(const Bend &t) const {
        return t.center == center && t.axis == axis && t.angle == angle;
    }

    /// Inequality operator.
    bool operator!=(const Bend &b) const { return ! (b == *this); }

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};
