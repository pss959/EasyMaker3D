#include "Math/Snap2D.h"

#include <cmath>

#include <ion/math/angleutils.h>
#include <ion/math/vectorutils.h>

#include "Math/Linear.h"

// XXXX
static Vector2f GetVector_(Snap2D::Direction direction) {
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

Snap2D::Direction Snap2D::GetSnapDirection(const Point2f &from_pos,
                                           const Point2f &to_pos,
                                           const Anglef &tolerance_angle) {
    // If the two points are too close together, can't do this.
    const Vector2f diff = to_pos - from_pos;
    if (ion::math::Length(diff) < .001f)
        return Direction::kNone;

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
            dir = dp.direction;
            break;
        }
    }
    return dir;
}

void Snap2D::SnapPointToDirection(Direction direction,
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

Snap2D::Result Snap2D::SnapPointBetween(const Point2f &p0, const Point2f &p1,
                                        const Anglef &tolerance_angle,
                                        Point2f &point_to_snap) {
    const Direction dir0 = GetSnapDirection(p0, point_to_snap, tolerance_angle);
    const Direction dir1 = GetSnapDirection(p1, point_to_snap, tolerance_angle);
    Result result = Result::kNeither;

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
        SnapPointToDirection(dir0, p0, point_to_snap);
        result = Result::kPoint0;
    }
    else if (dir1 != Direction::kNone) {
        SnapPointToDirection(dir1, p1, point_to_snap);
        result = Result::kPoint1;
    }

    return result;
}
