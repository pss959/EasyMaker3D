#include "Frustum.h"

#include <ion/math/angleutils.h>
#include <ion/math/rotation.h>

#include "SG/Math.h"

SG::Ray Frustum::BuildRay(const SG::Point2f &pt) {
    // Ignore position and orientation for now; assume the view point is at the
    // origin and the direction is -Z. Use the FOV angles to get the lower-left
    // and upper-right corners of the image rectangle in the near plane.
    // All values follow this form:
    //    tan(-fov_left) = left_x / near => left_x = near * tan(-fov_left).
    SG::Point2f ll(near * ion::math::Tangent(-fov_left),
                   near * ion::math::Tangent(-fov_down));
    SG::Point2f ur(near * ion::math::Tangent(fov_right),
                   near * ion::math::Tangent(fov_up));
    SG::Point2f rect_pt = ll + pt * ur;

    // The ray passes from the eyepoint (Frustum position) through the image
    // rectangle point. Use this to compute the canonical direction.
    SG::Point3f  image_pt(rect_pt[0], rect_pt[1], near);
    SG::Vector3f direction = ion::math::Normalized(image_pt - position);

    // Rotate the direction by the orientation.
    direction = orientation * direction;

    // Move from the eyepoint to the near plane along the direction to get the
    // ray origin.
    SG::Point3f origin = position + near * direction;
    return SG::Ray(origin, direction);
}
