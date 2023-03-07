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

    /// Result from calling SnapPointBetween().
    enum class Result { kNeither, kPoint0, kPoint1, kBoth };

    /// If the line segment from \p from_pos to \p to_pos is within the \p
    /// tolerance_angle of any principal direction, this returns that
    /// direction. If not, it returns Direction::kNone,
    static Direction GetSnapDirection(const Point2f &from_pos,
                                      const Point2f &to_pos,
                                      const Anglef &tolerance_angle);

    /// Modifies \p point_to_snap to snap to the given \p direction relative to
    /// \p fixed_point.
    static void SnapPointToDirection(Direction direction,
                                     const Point2f &fixed_point,
                                     Point2f &point_to_snap);

    /// If \p point_to_snap is within \p tolerance_angle of a principal
    /// direction from either or both fixed points, this snaps it to that
    /// direction. Returns a Result enum indicating what was snapped.
    static Result SnapPointBetween(const Point2f &p0, const Point2f &p1,
                                   const Anglef &tolerance_angle,
                                   Point2f &point_to_snap);
};
