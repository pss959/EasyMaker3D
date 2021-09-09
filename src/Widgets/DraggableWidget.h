#pragma once

#include "Assert.h"
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

  protected:
    //! Saves a path to this DraggableWidget for use in converting between
    //! coordinate systems. This can be called in StartDrag() to save the path
    //! for future use while dragging. This asserts if this widget is not in
    //! the path in the DragInfo.
    void SavePathToThis(const DragInfo &info) {
        path_to_this_ = info.hit.path.GetSubPath(*this);
    }

    //! \name Transformation Helper Functions
    //! These functions help convert between coordinate systems. They assume
    //! that SavePathToThis() has been called with a valid path. Local
    //! coordinates are defined as the coordinates at the tail of the path
    //! (this widget). They can be converted to and from the coordinate system
    //! at the root of the path.
    //!@{

    //! Transforms a point from local coordinates.
    Point3f FromLocal(const Point3f &p) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.FromLocal(p);
    }

    //! Transforms a vector from local coordinates.
    Vector3f FromLocal(const Vector3f &v) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.FromLocal(v);
    }

    //! Transforms a point to local coordinates.
    Point3f ToLocal(const Point3f &p) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.ToLocal(p);
    }

    //! Transforms a vector to local coordinates.
    Vector3f ToLocal(const Vector3f &v) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.ToLocal(v);
    }

    //!@}

  private:
    //! Path to this widget saved in SavePathToThis;
    SG::NodePath path_to_this_;
};
