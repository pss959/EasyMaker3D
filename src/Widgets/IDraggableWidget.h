#pragma once

#include "Math/Types.h"

//! IDraggableWidget is an abstract interface class for widgets that implement
//! interactive dragging. There are two modes of dragging: ray-based and
//! grip-based.  Ray-based drags are performed with the mouse or with
//! laser-pointer/pinch of a VR controller. Grip-based drags are always done
//! with a controller.
//! \ingroup Widgets
class IDraggableWidget {
  public:
    //! The DragInfo struct packages up information about a drag operation on
    //! an interactive object in the scene.
    struct DragInfo {
        //! The starting point of the drag in world coordinates.
        Point3f world_start_point;

        //! Set to true if the drag is a grip drag.
        bool    is_grip_drag = false;

        //! The current ray in world coordinates.
        Ray     world_ray;

        //! True if currently in alternate input mode.
        bool    is_alternate_mode = false;
    };

    //! StartDrag() begins a drag operation with the given DragInfo.
    virtual void StartDrag(const DragInfo &info) = 0;

    //! Drag() continues a drag operation.
    virtual void Drag(const DragInfo &info) = 0;

    //! EndDrag() finishes the drag operation.
    virtual void EndDrag() = 0;
};
