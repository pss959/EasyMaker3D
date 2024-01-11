#pragma once

#include "Math/Ray.h"
#include "Math/Types.h"

/// A Frustum struct represents a view frustum used to view a scene. It also
/// contains the Viewport being viewed in for convenience.
///
/// \ingroup Math
struct Frustum {
    Frustum();

    /// Viewport used for the view.
    Viewport  viewport;

    /// Position of the frustum view point. The default is (0,0,10).
    Point3f   position{ 0, 0, 10 };

    ///< Rotation of the frustum from its canonical orientation: looking along
    /// -Z with +Y as the up direction.
    Rotationf orientation;

    /// \name Field of View Angles
    /// These four angles define the field of view. The left and down angles
    /// are typically negative. Note that for VR, the field is not necessarily
    /// symmetric. The default value is -30 degrees for left and down and +30
    /// degrees for right and up.
    ///@{
    Anglef    fov_left;
    Anglef    fov_right;
    Anglef    fov_down;
    Anglef    fov_up;
    ///@}

    /// Distance to near plane from the view point along the view direction.
    /// The default is .01. Note that the name "near" is still reserved by the
    /// compiler on Windows.
    float     pnear;

    /// Distance to far plane from the view point along the view direction. The
    /// default is 20. Note that the name "far" is still reserved by the
    /// compiler on Windows.
    float     pfar;

    /// Convenience that sets the FOV angles to be symmetric based on an FOV
    /// angle and an aspect ratio. The FOV is applied to the larger dimension.
    void SetSymmetricFOV(const Anglef &fov, float aspect);

    /// Convenience that sets the FOV angles from half-angle tangents. This is
    /// what OpenVR supplies.
    void SetFromTangents(float left, float right, float down, float up);

    /// Convenience that returns the main view direction.
    Vector3f GetViewDirection() const {
        return orientation * -Vector3f::AxisZ();
    }

    /// Returns the 2D image rectangle based on the field of view.
    Range2f GetImageRect() const;

    /// Constructs a Ray through the given normalized point on the image
    /// rectangle (in the near plane). (0,0) is the lower-left corner of the
    /// rectangle.
    Ray BuildRay(const Point2f &pt) const;

    /// Projects the given 3D point onto the near plane and returns the result
    /// in normalized coordinates in the image rectangle.
    Point2f ProjectToImageRect(const Point3f &pt) const;

    /// Converts to a string to help with debugging.
    Str ToString() const;
};
