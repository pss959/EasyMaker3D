#include "View.h"

#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>
#include <ion/math/vector.h>

#include "Frustum.h"

using ion::math::Matrix4f;

float View::GetAspectRatio() const {
    const auto &size = viewport_.GetSize();
    return static_cast<float>(size[0]) / size[1];
}

Matrix4f View::GetProjectionMatrix() const {
    const float tan_l = tanf(frustum_.fov_left.Radians());
    const float tan_r = tanf(frustum_.fov_right.Radians());
    const float tan_u = tanf(frustum_.fov_up.Radians());
    const float tan_d = tanf(frustum_.fov_down.Radians());

    const float tan_lr = tan_r - tan_l;
    const float tan_du = tan_u - tan_d;

    const float near = frustum_.near;
    const float far  = frustum_.far;
    return Matrix4f(
        2 / tan_lr, 0, (tan_r + tan_l) / tan_lr, 0,
        0, 2 / tan_du, (tan_u + tan_d) / tan_du, 0,
        0, 0, -(far + near) / (far - near), -(2 * far * near) / (far - near),
        0, 0, -1, 0);
}

Matrix4f View::GetViewMatrix() const {
    return ion::math::RotationMatrixH(-frustum_.orientation) *
        ion::math::TranslationMatrix(-frustum_.position);
}
