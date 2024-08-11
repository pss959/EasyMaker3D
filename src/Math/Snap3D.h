//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Dimensionality.h"
#include "Math/Types.h"

struct Bounds;

/// The Snap3D class implements snapping of 3D points to fixed values.
///
/// \ingroup Math
class Snap3D {
  public:
    /// Snaps point \p to the edges of a Bounds. For each of the 3 dimensions,
    /// if \p p is within the \p tolerance in that dimension of the minimum,
    /// maximum, or center values of \p bounds in that dimension, it is moved
    /// to that value. This returns a Dimensionality indicating which
    /// dimensions were snapped.
    static Dimensionality SnapToBounds(const Bounds &bounds, Point3f &p,
                                       const Vector3f &tolerance);
};
