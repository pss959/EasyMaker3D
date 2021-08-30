#pragma once

#include <string>

#include "SG/Math.h"

//! A Frustum struct represents a view frustum used to view a scene. It acts as
//! a go-between data container to transfer projection and view information
//! from an IViewer to a View.
struct Frustum {
    //! Position of the frustum view point. The default is (0,0,10).
    SG::Point3f   position{ 0, 0, 10 };

    //!< Rotation of the frustum from its canonical orientation: looking along
    //! -Z with +Y as the up direction.
    SG::Rotationf orientation;

    //! \name Field of View Angles
    //! These four angles define the field of view. The left and down angles
    //! are typically negative. Note that for VR, the field is not necessarily
    //! symmetric. The default value is -30 degrees for left and down and +30
    //! degrees for right and up.
    //!@{
    SG::Anglef fov_left  = SG::Anglef::FromDegrees(-30);
    SG::Anglef fov_right = SG::Anglef::FromDegrees(30);
    SG::Anglef fov_down  = SG::Anglef::FromDegrees(-30);
    SG::Anglef fov_up    = SG::Anglef::FromDegrees(30);
    //!@}

    //! Distance to near plane from the view point along the view
    //! direction. The default is .01.
    float near = .01f;

    //! Distance to far plane from the view point along the view direction. The
    //! default is 20.
    float far = 20.f;

    //! Convenience that sets the FOV angles to be symmetric based on a
    //! vertical FOV angle and an aspect ratio.
    void SetSymmetricFOV(const SG::Anglef &vfov, float aspect);

    //! Constructs an SG::Ray through the given normalized point on the image
    //! rectangle (in the near plane). (0,0) is the lower-left corner of the
    //! rectangle.
    SG::Ray BuildRay(const SG::Point2f &pt);

    //! Converts to a string to help with debugging.
    std::string ToString() const;
};
