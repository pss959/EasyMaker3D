﻿#pragma once

#include "Math/Types.h"

/// The ColorTool displays a ring that allows the user to select a color for a
/// Model. The static ColorRing class represents that ring and allows points on
/// it to be correlated with colors. This assumes the ring is centered on (0,0)
/// with an outer radius of Defaults::kColorRingOuterRadius and an inner radius
/// of Defaults::kColorRingInnerRadius.
///
/// \ingroup Math
class ColorRing {
  public:
    /// Returns the Color associated with the given point. The hue is
    /// determined by the angle of the point around the center. The saturation
    /// and value are determined by the distance from the center, clamped to
    /// the inner and outer radii.
    static Color   GetColorForPoint(const Point2f &point);

    /// Returns the point on the ring corresponding to the given Color. The
    /// saturation and value of the Color are clamped if necessary to ensure
    /// that the resulting point is on the ring.
    static Point2f GetPointForColor(const Color &color);
};
