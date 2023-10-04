#include "Math/Snap2D.h"

#include <cmath>
#include <numbers>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Util/Enum.h"

Snap2D::Result Snap2D::SnapPointBetween(const Point2f &p0, const Point2f &p1,
                                        Point2f &point_to_snap) {
    Result result = Result::kNeither;

    const Direction_ dir0 = GetSnapDirection_(p0, point_to_snap);
    const Direction_ dir1 = GetSnapDirection_(p1, point_to_snap);

    // If snapping to both directions, have to find their intersection.
    if (dir0 != Direction_::kNone && dir1 != Direction_::kNone) {
        const auto v0 = GetVector_(dir0);
        const auto v1 = GetVector_(dir1);
        // If directions are opposite, find the closest point on the line from
        // p0 to p1 to point_to_snap.
        if (v0 == -v1) {
            const Point3f c = GetClosestPointOnLine(Point3f(point_to_snap, 0),
                                                    Point3f(p0, 0),
                                                    Vector3f(v0, 0));
            point_to_snap = ToPoint2f(c);
            result = Result::kBoth;
        }
        else {
            // Use the 3D function to make this easier. If there is no
            // intersection, do not snap at all.
            Point3f c0, c1;
            if (GetClosestLinePoints(Point3f(p0, 0), Vector3f(v0, 0),
                                     Point3f(p1, 0), Vector3f(v1, 0), c0, c1)) {
                point_to_snap = ToPoint2f(c0);
                result = Result::kBoth;
            }
        }
    }
    else if (dir0 != Direction_::kNone) {
        SnapPointToDirection_(dir0, p0, point_to_snap);
        result = Result::kPoint0;
    }
    else if (dir1 != Direction_::kNone) {
        SnapPointToDirection_(dir1, p1, point_to_snap);
        result = Result::kPoint1;
    }

    return result;
}

Snap2D::Direction_ Snap2D::GetSnapDirection_(const Point2f &p0,
                                             const Point2f &p1) {
    // If the two points are too close together, can't do this.
    const Vector2f vec = p1 - p0;
    if (ion::math::Length(vec) < .001f)
        return Direction_::kNone;

    // Compare with principal directions.
    Direction_ snap_direction = Direction_::kNone;
    for (const auto dir: Util::EnumValues<Direction_>()) {
        if (dir == Direction_::kNone)
            continue;
        // Find the angle between the vector from p0 to p1 and the vector for
        // this direction.
        const Vector2f dir_vec = GetVector_(dir);
        const Anglef angle = RotationAngle(
            Rotationf::RotateInto(Vector3f(vec, 0), Vector3f(dir_vec, 0)));
        if (std::abs(angle.Radians()) <= tolerance_angle_.Radians()) {
            snap_direction = dir;
            break;
        }
    }
    return snap_direction;
}

void Snap2D::SnapPointToDirection_(Direction_ direction,
                                   const Point2f &fixed_point,
                                   Point2f &point_to_snap) {
    switch (direction) {
        using enum Snap2D::Direction_;
      case kN: case kS:
        point_to_snap[0] = fixed_point[0];
        break;
      case kE: case kW:
        point_to_snap[1] = fixed_point[1];
        break;
      case kNE: case kNW: case kSE: case kSW: {
          // Compute the side of a 45-degree right triangle.
          const float len = ion::math::Length(point_to_snap - fixed_point) /
              std::numbers::sqrt2;
          point_to_snap = fixed_point + len * GetVector_(direction);
          break;
      }
      default: break;  // LCOV_EXCL_LINE [cannot happen]
    }
}

Vector2f Snap2D::GetVector_(Direction_ direction) {
    Vector2f v;
    switch (direction) {
        using enum Snap2D::Direction_;
      case kN:  v.Set( 0,  1); break;
      case kS:  v.Set( 0, -1); break;
      case kE:  v.Set( 1,  0); break;
      case kW:  v.Set(-1,  0); break;
      case kNE: v.Set( 1,  1); break;
      case kNW: v.Set(-1,  1); break;
      case kSE: v.Set( 1, -1); break;
      case kSW: v.Set(-1, -1); break;
      default:  v.Set( 0,  0); break;  // LCOV_EXCL_LINE [cannot happen]
    }
    return v;
}
