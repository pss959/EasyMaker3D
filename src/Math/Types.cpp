#include "Math/Types.h"

#include <ion/math/vectorutils.h>

#include "Util/String.h"

// ----------------------------------------------------------------------------
// Plane functions.
// ----------------------------------------------------------------------------

Plane::Plane(const Point3f &point, const Vector3f &norm) {
    normal   = ion::math::Normalized(norm);
    distance = ion::math::Dot(normal, point - Point3f::Zero());
}

Plane::Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2) {
    normal   = ion::math::Normalized(ion::math::Cross(p1 - p0, p2 - p0));
    distance = ion::math::Dot(normal, Vector3f(p0));
}

std::string Plane::ToString() const {
    return ("PL [n="  + Util::ToString(normal)  +
            " d="     + Util::ToString(distance) +
            "]");
}

// ----------------------------------------------------------------------------
// Ray functions.
// ----------------------------------------------------------------------------

std::string Ray::ToString() const {
    return ("RAY [o="  + Util::ToString(origin)  +
            " d="      + Util::ToString(direction) +
            "]");
}

// ----------------------------------------------------------------------------
// Frustum functions.
// ----------------------------------------------------------------------------

void Frustum::SetSymmetricFOV(const Anglef &vfov, float aspect) {
    fov_right   = .5f * aspect * vfov;
    fov_up      = .5f * vfov;
    fov_left    = -fov_right;
    fov_down    = -fov_up;
}

Ray Frustum::BuildRay(const Point2f &pt) {
    // Ignore position and orientation for now; assume the direction is -Z. Use
    // the FOV angles to get the lower-left and upper-right corners of the
    // image rectangle in the near plane.  All values follow this form:
    // tan(-fov_left) = left_x / near => left_x = near * tan(-fov_left).
    Point2f ll(near * ion::math::Tangent(fov_left),
               near * ion::math::Tangent(fov_down));
    Point2f ur(near * ion::math::Tangent(fov_right),
               near * ion::math::Tangent(fov_up));
    Point2f rect_pt = ll + Vector2f(pt) * (ur - ll);

    // Use this to compute the point in the canonical image plane.  This point
    // gives us the unrotated direction relative to the view position. The
    // length of this vector is the distance from the eyepoint to the ray
    // origin;
    Vector3f direction(rect_pt[0], rect_pt[1], -near);
    const float distance = ion::math::Length(direction);

    // Rotate the normalized direction by the orientation.
    direction = orientation * ion::math::Normalized(direction);

    // Use the distance and the correct direction to get the ray origin.
    return Ray(position + distance * direction, direction);
}

std::string Frustum::ToString() const {
    return ("FR [pos="  + Util::ToString(position) +
            " or="      + Util::ToString(orientation) +
            " fov=(l:"  + Util::ToString(fov_left) +
            " r:"       + Util::ToString(fov_right) +
            " u:,"      + Util::ToString(fov_up) +
            " d:,"      + Util::ToString(fov_down) +
            ") nr="     + Util::ToString(near) +
            " fr="      + Util::ToString(far) +
            "]");
}
