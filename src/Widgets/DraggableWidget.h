#pragma once

#include "Math/Types.h"
#include "SG/Hit.h"
#include "Widgets/Widget.h"

//! IDraggableWidget is an abstract base class for widgets that implement
//! interactive dragging. There are two modes of dragging: ray-based and
//! grip-based.  Ray-based drags are performed with the mouse or with
//! laser-pointer/pinch of a VR controller. Grip-based drags are always done
//! with a controller.
//! \ingroup Widgets
class DraggableWidget : public Widget {
  public:
    //! The DragInfo struct packages up information about a drag operation on
    //! an interactive object in the scene.
    struct DragInfo {
        //! The SG::Hit that started the drag. The point in the Hit is the
        //! starting point of the drag in world coordinates and the ray is the
        //! starting ray of the drag in world coordinates.
        SG::Hit hit;

        //! True if the drag is a grip drag.
        bool    is_grip_drag = false;

        //! True if currently in alternate input mode.
        bool    is_alternate_mode = false;
    };

    //! \name Dragging functions
    //! Derived classes must implement these for dragging.
    //!@{

    //! StartDrag() begins a drag operation with the given DragInfo.
    virtual void StartDrag(const DragInfo &info) = 0;

    //! Drag() continues a drag operation.
    virtual void ContinueDrag(const DragInfo &info) = 0;

    //! EndDrag() finishes the drag operation.
    virtual void EndDrag() = 0;

    //!@}
};
