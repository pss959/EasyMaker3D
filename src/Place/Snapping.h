#pragma once

#include "Math/Dimensionality.h"
#include "Math/Types.h"

/// \file
/// Snapping utility functions
///
/// \ingroup Place

/// For each of the 3 dimensions, if point \p is within the \p tolerance in
/// that dimension to the minimum, maximum, or center values of \p bounds in
/// that dimension, it is moved to that value. This returns a Dimensionality
/// indicating which dimensions were snapped.
Dimensionality SnapToBounds(const Bounds &bounds, Point3f &p,
                            const Vector3f &tolerance);
