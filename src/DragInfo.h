#pragma once

#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/NodePath.h"

/// The DragInfo struct packages up information about a drag operation on
/// a Widget in the scene. It is used for derived DraggableWidget classes and
/// also for placing targets. An instance of this is set up by the MainHandler
/// for both pointer-based and grip drags.
struct DragInfo {
    // ------------------------------------
    /// \name Common fields.
    /// These fields are available for any type of drag operation.
    ///@{

    /// True if the drag is a grip drag.
    bool         is_grip = false;

    /// True if currently in alternate input mode.
    bool         is_alternate_mode = false;

    /// Linear precision to use for the drag (if the Widget uses it).
    float        linear_precision = 0;

    /// Angular precision (in degrees) to use for the drag (if the Widget
    /// uses it).
    float        angular_precision = 0;

    /// Path to the DraggableWidget being dragged. This does not change
    /// throughout the drag.
    SG::NodePath path_to_widget;

    ///@}

    // ------------------------------------
    /// \name Pointer-based drag fields.
    /// These fields are available only when is_grip is false.
    ///@{

    /// The pointer ray (in world coordinates).
    Ray          ray;

    /// SG::Hit representing the intersection of the pointer ray.
    SG::Hit      hit;

    ///@}

    // ------------------------------------
    /// \name Grip drag fields.
    /// These fields are available only when is_grip is true.
    ///@{

    /// Controller position in world coordinates.
    Point3f      grip_position;

    /// Controller orientation.
    Rotationf    grip_orientation;

    /// Convenience function that converts grip_position to local coordinates
    /// of the Widget.
    Point3f GetLocalGripPosition() const {
        return path_to_widget.ToLocal(grip_position);
    }

    ///@}
};
