#include "Math/Frustum.h"

#include <ion/math/angleutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Util/Tuning.h"

Frustum::Frustum() {
    const Anglef fov_angle = Anglef::FromDegrees(TK::kFOVHalfAngle);
    fov_left  = fov_down  = -fov_angle;
    fov_right = fov_up    =  fov_angle;

    pnear = TK::kNearDistance;
    pfar  = TK::kFarDistance;
}

void Frustum::SetSymmetricFOV(const Anglef &fov, float aspect) {
    // Use the specified FOV for the larger dimension and compute the FOV for
    // the other dimension.
    using ion::math::ArcTangent;
    using ion::math::Tangent;
    Anglef half_v_fov, half_h_fov;
    if (aspect >= 1.f) {
        // Wider than tall; use given FOV in horizontal dimension.
        half_h_fov = .5f * fov;
        half_v_fov = ArcTangent(Tangent(half_h_fov) / aspect);
    }
    else {
        // Taller than wide; use given FOV in vertical dimension.
        half_v_fov = .5f * fov;
        half_h_fov = ArcTangent(Tangent(half_v_fov) * aspect);
    }

    fov_left    = -half_h_fov;
    fov_right   =  half_h_fov;
    fov_down    = -half_v_fov;
    fov_up      =  half_v_fov;
}

void Frustum::SetFromTangents(float left, float right, float down, float up) {
    using ion::math::ArcTangent;
    fov_left  = ArcTangent(left);
    fov_right = ArcTangent(right);
    fov_down  = ArcTangent(down);
    fov_up    = ArcTangent(up);
}

Range2f Frustum::GetImageRect() const {
    // Use the FOV angles to get the lower-left and upper-right corners of the
    // image rectangle in the near plane.  All values follow this form:
    // tan(-fov_left) = left_x / near => left_x = near * tan(-fov_left).
    const Point2f ll(pnear * ion::math::Tangent(fov_left),
                     pnear * ion::math::Tangent(fov_down));
    const Point2f ur(pnear * ion::math::Tangent(fov_right),
                     pnear * ion::math::Tangent(fov_up));
    return Range2f(ll, ur);
}

Ray Frustum::BuildRay(const Point2f &pt) const {
    const Range2f image_rect = GetImageRect();
    const Point2f rect_pt =
        image_rect.GetMinPoint() + Vector2f(pt) * image_rect.GetSize();

    // Use this to compute the point in the canonical image plane.  This point
    // gives us the unrotated direction relative to the view position. The
    // length of this vector is the distance from the eyepoint to the ray
    // origin;
    Vector3f direction(rect_pt[0], rect_pt[1], -pnear);
    const float distance = ion::math::Length(direction);

    // Rotate the normalized direction by the orientation.
    direction = orientation * ion::math::Normalized(direction);

    // Use the distance and the correct direction to get the ray origin.
    return Ray(position + distance * direction, direction);
}

Point2f Frustum::ProjectToImageRect(const Point3f &pt) const {
    // Apply the projection and view matrices and convert to normalized image
    // rectangle coordinates.
    const Matrix4f m = GetProjectionMatrix(*this) * GetViewMatrix(*this);
    const Point3f  p = ion::math::ProjectPoint(m, pt);
    return Point2f(.5f + .5f * p[0],
                   .5f + .5f * p[1]);
}

Str Frustum::ToString() const {
    return ("FR [vp="  + ToStr(viewport) +
            " p="      + ToStr(position) +
            " o="      + ToStr(orientation) +
            " fov=(l:" + ToStr(fov_left) +
            " r:"      + ToStr(fov_right) +
            " u:"      + ToStr(fov_up) +
            " d:"      + ToStr(fov_down) +
            ") n="     + ToStr(pnear) +
            " f="      + ToStr(pfar) +
            "]");
}
