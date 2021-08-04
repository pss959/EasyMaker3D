#pragma once

#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/rotation.h>

//! This struct represents a view used to render a scene.
struct View {
    //! Rectangle definining the viewport.
    ion::math::Range2i   viewport_rect;

    //! Matrix defining the camera projection.
    ion::math::Matrix4f  projection_matrix = ion::math::Matrix4f::Identity();

    //! Matrix defining the camera view.
    ion::math::Matrix4f  view_matrix = ion::math::Matrix4f::Identity();

    //! Additional camera rotation, which is applied to the camera view.
    ion::math::Rotationf camera_rotation;
};
