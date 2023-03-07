#include "Math/Snap2D.h"

#include <cmath>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Util/Enum.h"

Snap2D::Result Snap2D::SnapPointBetween(const Point2f &p0, const Point2f &p1,
                                        Point2f &point_to_snap) {
    Result result = Result::kNeither;

    const Direction dir0 = GetSnapDirection_(p0, point_to_snap);
    const Direction dir1 = GetSnapDirection_(p1, point_to_snap);

    // If snapping to both directions, have to find their intersection.
    if (dir0 != Direction::kNone && dir1 != Direction::kNone) {
        // Use the 3D function to make this easier. If there is no
        // intersection, do not snap at all.
        Point3f c0, c1;
        if (GetClosestLinePoints(Point3f(p0, 0), Vector3f(GetVector_(dir0), 0),
                                 Point3f(p1, 0), Vector3f(GetVector_(dir1), 0),
                                 c0, c1)) {

            point_to_snap = ToPoint2f(c0);
            result = Result::kBoth;
        }
    }
    else if (dir0 != Direction::kNone) {
        SnapPointToDirection_(dir0, p0, point_to_snap);
        result = Result::kPoint0;
    }
    else if (dir1 != Direction::kNone) {
        SnapPointToDirection_(dir1, p1, point_to_snap);
        result = Result::kPoint1;
    }

    return result;
}

Snap2D::Direction Snap2D::GetSnapDirection_(const Point2f &p0,
                                            const Point2f &p1) {
    // If the two points are too close together, can't do this.
    const Vector2f vec = p1 - p0;
    if (ion::math::Length(vec) < .001f)
        return Direction::kNone;

    // Compare with principal directions.
    Direction snap_direction = Direction::kNone;
    for (const auto dir: Util::EnumValues<Direction>()) {
        if (dir == Direction::kNone)
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

void Snap2D::SnapPointToDirection_(Direction direction,
                                   const Point2f &fixed_point,
                                   Point2f &point_to_snap) {
    switch (direction) {
      case Snap2D::Direction::kN:
      case Snap2D::Direction::kS:
        point_to_snap[0] = fixed_point[0];
        break;
      case Snap2D::Direction::kE:
      case Snap2D::Direction::kW:
        point_to_snap[1] = fixed_point[1];
        break;
      case Snap2D::Direction::kNE:
      case Snap2D::Direction::kNW:
      case Snap2D::Direction::kSE:
      case Snap2D::Direction::kSW: {
          // Compute the side of a 45-degree right triangle.
          const float len =
              ion::math::Length(point_to_snap - fixed_point) / std::sqrt(2);
          point_to_snap = fixed_point + len * GetVector_(direction);
          break;
      }
      default:
        break;
    }
}

Vector2f Snap2D::GetVector_(Direction direction) {
    Vector2f v;
    switch (direction) {
      case Snap2D::Direction::kN:  v.Set( 0,  1); break;
      case Snap2D::Direction::kS:  v.Set( 0, -1); break;
      case Snap2D::Direction::kE:  v.Set( 1,  0); break;
      case Snap2D::Direction::kW:  v.Set(-1,  0); break;
      case Snap2D::Direction::kNE: v.Set( 1,  1); break;
      case Snap2D::Direction::kNW: v.Set(-1,  1); break;
      case Snap2D::Direction::kSE: v.Set( 1, -1); break;
      case Snap2D::Direction::kSW: v.Set(-1, -1); break;
      default:                     v.Set( 0,  0); break;
    }
    return v;
}
