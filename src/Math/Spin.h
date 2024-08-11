//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"

/// An Spin struct defines a spin by some angle around a general 3D axis, which
/// is specified as a center point and direction vector. There is also an
/// optional offset that is applied in the direction of the axis for each 360
/// degrees of spin.
///
/// \ingroup Math
struct Spin {
    Point3f  center{0, 0, 0};  ///< Axis center; defaults to origin.
    Vector3f axis{0, 1, 0};    ///< Axis direction; defaults to +Y axis.
    Anglef   angle;            ///< Angle; defaults to 0.
    float    offset = 0;       ///< Offset per 360 degrees; defaults to 0.

    /// Equality operator.
    bool operator==(const Spin &s) const = default;

    /// Converts to a string to help with debugging.
    Str ToString() const;
};
