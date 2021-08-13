#pragma once

#include <string>

#include <ion/math/angle.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include "Util.h"

//! This struct represents a perspective camera used to view a scene.
struct Camera {
    //! Struct definining field of view angles. The left and down angles are
    //! negative for a symmetric view. Note that for VR, the field is not
    //! necessarily symmetric.
    struct FOV {
        ion::math::Anglef left;
        ion::math::Anglef right;
        ion::math::Anglef up;
        ion::math::Anglef down;

        //! The default constructor sets reasonable values.
        FOV() {
            right = up   = ion::math::Anglef::FromDegrees(30.f);
            left  = down = -right;
        }

        //! Constructor that sets up a symmetric FOV using the given full
        //! vertical angle and aspect ratio.
        FOV(const ion::math::Anglef &vfov, float aspect) {
            up    = .5f * vfov;
            down  = -up;
            right = aspect * up;
            left  = -right;
        }
    };

    //! Position of the camera in 3D coordinates.
    ion::math::Vector3f position;

    //! Rotation applied to the camera from its canonical orientation: looking
    //! along -Z with +Y as the up direction.
    ion::math::Rotationf orientation;

    //! Field of view angles.
    FOV fov;

    //! Distance to near plane.
    float near;

    //!< Distance to far plane.
    float far;

    //! The default constructor sets some reasonable values.
    Camera() :  position(0.f, 0.f, -10.f), near(.01f),  far(20.f) {}

    //! Converts to a string for printing.
    const std::string ToString() const {
        return ("Cam [pos=" + Util::ToString(position) +
                " or="      + Util::ToString(orientation) +
                " fov=(l:"  + Util::ToString(fov.left) +
                " r:"       + Util::ToString(fov.right) +
                " u:,"      + Util::ToString(fov.up) +
                " d:,"      + Util::ToString(fov.down) +
                ") nr="     + Util::ToString(near) +
                " fr="       + Util::ToString(far) +
                "]");
    }
};
