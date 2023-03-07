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

    /// Sets the tolerance angle to use for snapping.
    void SetToleranceAngle(const Anglef &angle) { tolerance_angle_ = angle; }

    /// Returns the tolerance angle used for snapping.
    const Anglef & GetToleranceAngle() const { return tolerance_angle_; }

    /// If \p point_to_snap is within the tolerance angle of a principal
    /// direction from either or both \p p0 and \p p1, this snaps it to that
    /// direction. Returns a Result enum indicating what was snapped.
    Result SnapPointBetween(const Point2f &p0, const Point2f &p1,
                            Point2f &point_to_snap);

  private:
    Anglef tolerance_angle_;

    /// If the line segment from \p p0 to \p p1 is within the tolerance angle
    /// of any principal direction, this returns that direction. If not, it
    /// returns Direction::kNone,
    Direction GetSnapDirection_(const Point2f &p0, const Point2f &p1);

    /// Modifies \p point_to_snap to snap to the given \p direction relative to
    /// \p fixed_point.
    void SnapPointToDirection_(Direction direction, const Point2f &fixed_point,
                               Point2f &point_to_snap);

    /// Returns the 2D vector corresponding to a Direction.
    static Vector2f GetVector_(Direction direction);
};
