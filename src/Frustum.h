#pragma once

#include <string>

#include <ion/math/angle.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include "Util/String.h"

//! A Frustum struct represents a view frustum used to view a scene. It acts as
//! a go-between data container to transfer projection and view information
//! from an IViewer to a View.
struct Frustum {
    //! Position of the frustum view point. The default is (0,0,-10).
    ion::math::Vector3f  position{ 0, 0, -10 };

    //!< Rotation of the frustum from its canonical orientation: looking along
    //! -Z with +Y as the up direction.
    ion::math::Rotationf orientation;

    //! \name Field of View Angles
    //! These four angles define the field of view. The left and down angles
    //! are typically negative. Note that for VR, the field is not necessarily
    //! symmetric. The default value is -30 degrees for left and down and +30
    //! degrees for right and up.
    //!@{
    ion::math::Anglef fov_left  = ion::math::Anglef::FromDegrees(-30);
    ion::math::Anglef fov_right = ion::math::Anglef::FromDegrees(30);
    ion::math::Anglef fov_down  = ion::math::Anglef::FromDegrees(-30);
    ion::math::Anglef fov_up    = ion::math::Anglef::FromDegrees(30);
    //!@}

    //! Distance to near plane from the view point along the view
    //! direction. The default is .01.
    float near = .01f;

    //! Distance to far plane from the view point along the view direction. The
    //! default is 20.
    float far = 20.f;

    //! Converts to a string to help with debugging.
    std::string ToString() const {
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
};
