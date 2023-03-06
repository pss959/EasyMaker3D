#pragma once

#include "Math/Types.h"

/// The Snap2D class implements snapping of 2D points to horizontal, vertical,
/// and 45-degree diagonal directions.
///
/// \ingroup Math
class Snap2D {
  public:
    /// Directions to snap to.
    enum class Direction { kNone, kN, kE, kW, kS, kNE, kNW, kSE, kSW };

    /// If the line segment from @from_pos to @to_pos is within the
    /// @tolerance_angle of any principal direction, this returns that
    /// direction and sets @angle_off to the angle formed by the segment and
    /// that direction. If not, it returns Direction::kNone,
    static Direction GetSnapDirection(const Point2f &from_pos,
                                      const Point2f &to_pos,
                                      const Anglef &tolerance_angle,
                                      Anglef &angle_off);

    /// Modifies @point_to_snap to snap to the given @direction relative to
    /// @fixed_point.
    static void SnapPointToDirection(Direction direction,
                                     const Point2f &fixed_point,
                                     Point2f &point_to_snap);
};
