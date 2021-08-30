#include "Frustum.h"

#include <ion/math/angleutils.h>
#include <ion/math/rotation.h>

#include "SG/Math.h"
#include "Util/String.h"

void Frustum::SetSymmetricFOV(const SG::Anglef &vfov, float aspect) {
    fov_right   = .5f * aspect * vfov;
    fov_up      = .5f * vfov;
    fov_left    = -fov_right;
    fov_down    = -fov_up;
}

SG::Ray Frustum::BuildRay(const SG::Point2f &pt) {
    // Ignore position and orientation for now; assume the view point is at the
    // origin and the direction is -Z. Use the FOV angles to get the lower-left
    // and upper-right corners of the image rectangle in the near plane.
    // All values follow this form:
    //    tan(-fov_left) = left_x / near => left_x = near * tan(-fov_left).
    const float near_z = position[2] - near;
    SG::Point2f ll(near * ion::math::Tangent(fov_left),
                   near * ion::math::Tangent(fov_down));
    SG::Point2f ur(near * ion::math::Tangent(fov_right),
                   near * ion::math::Tangent(fov_up));
    SG::Point2f rect_pt = ll + SG::Vector2f(pt) * (ur - ll);

    // The ray passes from the eyepoint (Frustum position) through the image
    // rectangle point. Use this to compute the canonical direction.
    SG::Point3f  image_pt(rect_pt[0], rect_pt[1], near_z);
    SG::Vector3f direction = ion::math::Normalized(image_pt - position);

    // Rotate the direction by the orientation.
    direction = orientation * direction;

    // The ray origin is the point on the image plane.
    return SG::Ray(image_pt, direction);
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
