#pragma once

#include "App/CoordConv.h"
#include "Enums/Hand.h"
#include "Enums/Trigger.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/NodePath.h"
#include "Util/Assert.h"

/// The DragInfo struct packages up information about a drag operation on a
/// Widget in the scene. It is used for derived DraggableWidget classes and
/// also for placing targets. An instance of this is set up by the MainHandler
/// for pointer-based, grip, and touch drags.
struct DragInfo {
    // ------------------------------------
    /// \name Common fields.
    /// These fields are available for any type of drag operation.
    ///@{

    /// Trigger for drag in progress.
    Trigger      trigger = Trigger::kPointer;

    /// True if currently in modified input mode.
    bool         is_modified_mode = false;

    /// Linear precision to use for the drag (if the Widget uses it).
    float        linear_precision = 0;

    /// Angular precision (in degrees) to use for the drag (if the Widget
    /// uses it).
    float        angular_precision = 0;

    /// Path to the DraggableWidget being dragged. This does not change
    /// throughout the drag.
    SG::NodePath path_to_widget;

    /// Path from the root of the scene to the Stage. This can be used to
    /// convert to and from Stage coordinates.
    SG::NodePath path_to_stage;

    ///@}

    // ------------------------------------
    /// \name Pointer-based drag fields.
    /// These fields are available only when trigger is Trigger::kPointer.
    ///@{

    /// The pointer ray (in world coordinates).
    Ray          ray;

    /// SG::Hit representing the intersection of the pointer ray.
    SG::Hit      hit;

    ///@}

    // ------------------------------------
    /// \name Grip drag fields.
    /// These fields are available only when trigger is Trigger::kGrip.
    ///@{

    /// Hand used for grip dragging.
    Hand         grip_hand = Hand::kRight;

    /// Controller position in world coordinates.
    Point3f      grip_position;

    /// Controller orientation.
    Rotationf    grip_orientation;

    // ------------------------------------
    /// \name Touch-based drag fields.
    /// These fields are available only when trigger is Trigger::kTouch.
    ///@{

    /// Touch position in world coordinates.
    Point3f       touch_position;

    // ------------------------------------

    /// Convenience function that returns a matrix converting from world
    /// coordinates to stage coordinates.
    Matrix4f GetWorldToStageMatrix() const {
        return CoordConv(path_to_stage).GetRootToObjectMatrix();
    }

    /// Convenience function that returns a matrix converting from object
    /// coordinates of the hit object to stage coordinates. This works only for
    /// pointer-based drags.
    Matrix4f GetObjectToStageMatrix() const {
        ASSERT(trigger == Trigger::kPointer);
        return CoordConv(path_to_stage).GetRootToObjectMatrix() *
            CoordConv(hit.path).GetObjectToRootMatrix();
    }

    /// Convenience function that converts grip_position to local coordinates
    /// of the Widget.
    Point3f GetLocalGripPosition() const {
        return CoordConv(path_to_widget).RootToLocal(grip_position);
    }

    ///@}
};
