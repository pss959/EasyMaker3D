#pragma once

#include <string>

#include "Math/Types.h"

/// A Bend defines how to bend a mesh around an axis. There is an optional
/// offset that is applied to the mesh as it bends. The offset is expressed as
/// a signed distance in stage coordinates along the axis that represents how
/// much to move mesh points for each 360 degrees of bend. For example, an
/// offset of 20 means that a mesh that is bent 360 degrees will end up 20
/// units along the axis direction relative to where it would be with no
/// offset.
///
/// \ingroup Math
struct Bend {
    Point3f  center{0, 0, 0};  ///< Bend center; defaults to origin.
    Vector3f axis{0, 0, 1};    ///< Bend axis; defaults to +Z axis.
    Anglef   angle;            ///< Bend angle; defaults to 0.
    float    offset = 0;       ///< Offset per 360 degrees; defaults to 0.

    /// Equality operator.
    bool operator==(const Bend &t) const {
        return t.center == center && t.axis == axis &&
            t.angle == angle && t.offset == offset;
    }

    /// Inequality operator.
    bool operator!=(const Bend &b) const { return ! (b == *this); }

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};
