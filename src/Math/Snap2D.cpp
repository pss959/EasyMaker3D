#include "Math/Snap2D.h"

#include <cmath>

#include <ion/math/angleutils.h>
#include <ion/math/vectorutils.h>

#include "Math/Linear.h"

Snap2D::Direction Snap2D::GetSnapDirection(const Point2f &from_pos,
                                           const Point2f &to_pos,
                                           const Anglef &tolerance_angle,
                                           Anglef &angle_off) {
    // If the two points are too close together, can't do this.
    const Vector2f diff = to_pos - from_pos;
    if (ion::math::Length(diff) < .001f) {
        angle_off = Anglef();
        return Direction::kNone;
    }

    // Determine the angle of the new point around those points. This should be
    // in the range -180 to +180, with 0 to the right (east)
    const Anglef candidate_angle = ion::math::ArcTangent2(diff[1], diff[0]);

    // Compare with principal directions.
    struct DirPair_ { float degrees; Direction direction; };
    static const std::vector<DirPair_> dir_pairs = {
        {    0, Direction::kE  },
        {   45, Direction::kNE },
        {   90, Direction::kN  },
        {  135, Direction::kNW },
        {  180, Direction::kW  },
        { -180, Direction::kW  },
        { -135, Direction::kSW },
        {  -90, Direction::kS  },
        {  -45, Direction::kSE },
    };
    Direction dir = Direction::kNone;
    for (const auto &dp: dir_pairs) {
        const Anglef dir_angle = Anglef::FromDegrees(dp.degrees);
        if (AreClose(candidate_angle, dir_angle, tolerance_angle)) {
            angle_off = candidate_angle - dir_angle;
            dir = dp.direction;
            break;
        }
    }
    return dir;
}

void Snap2D::SnapPointToDirection(Direction direction,
                                  const Point2f &fixed_point,
                                  Point2f &point_to_snap) {
    // Snaps to a diagonal direction.
    auto diag = [&](int x_sign, int y_sign){
        // Compute the side of a 45-degree right triangle.
        const float len =
            ion::math::Length(point_to_snap - fixed_point) / std::sqrt(2);
        return fixed_point + Vector2f(x_sign * len, y_sign * len);
    };

    switch (direction) {
      case Snap2D::Direction::kN:
      case Snap2D::Direction::kS:  point_to_snap[0] = fixed_point[0]; break;
      case Snap2D::Direction::kE:
      case Snap2D::Direction::kW:  point_to_snap[1] = fixed_point[1]; break;
      case Snap2D::Direction::kNE: point_to_snap = diag( 1,  1); break;
      case Snap2D::Direction::kNW: point_to_snap = diag(-1,  1); break;
      case Snap2D::Direction::kSE: point_to_snap = diag( 1, -1); break;
      case Snap2D::Direction::kSW: point_to_snap = diag(-1, -1); break;
      default: break;
    }
}

Snap2D::Result Snap2D::SnapPointBetween(const Point2f &p0, const Point2f &p1,
                                        const Anglef &tolerance_angle,
                                        Point2f &point_to_snap) {
    // Snap to the closest direction around both points and see how close they
    // are to that direction.
    Anglef angle0, angle1;
    const Direction dir0 = GetSnapDirection(p0, point_to_snap,
                                            tolerance_angle, angle0);
    const Direction dir1 = GetSnapDirection(p1, point_to_snap,
                                            tolerance_angle, angle1);

    Result result = Result::kNeither;
    if (dir0 != Direction::kNone && dir1 != Direction::kNone) {
        // Snap to both, snapping first to the closer direction.
        if (std::abs(angle0.Radians()) <= std::abs(angle1.Radians())) {
            SnapPointToDirection(dir0, p0, point_to_snap);
            SnapPointToDirection(dir1, p1, point_to_snap);
        }
        else {
            SnapPointToDirection(dir1, p1, point_to_snap);
            SnapPointToDirection(dir0, p0, point_to_snap);
        }
        result = Result::kBoth;
    }
    else if (dir0 != Direction::kNone) {
        SnapPointToDirection(dir0, p0, point_to_snap);
        result = Result::kPoint0;
    }
    else if (dir1 != Direction::kNone) {
        SnapPointToDirection(dir1, p1, point_to_snap);
        result = Result::kPoint1;
    }

    return result;
}
