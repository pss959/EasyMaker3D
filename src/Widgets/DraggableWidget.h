#pragma once

#include <memory>

#include "Assert.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "Widgets/ClickableWidget.h"

/// DraggableWidget is an abstract base class for widgets that implement
/// interactive dragging. There are two modes of dragging: ray-based and
/// grip-based.  Ray-based drags are performed with the mouse or with
/// laser-pointer/pinch of a VR controller. Grip-based drags are always done
/// with a controller.
///
/// DraggableWidget is derived from ClickableWidget, since draggable widgets
/// should also implement clicking and/or alt-clicking to reset.
/// \ingroup Widgets
class DraggableWidget : public ClickableWidget {
  public:
    /// The DragInfo struct packages up information about a drag operation on
    /// an interactive object in the scene.
    struct DragInfo {
        /// The SG::Hit that started the drag. The point in the Hit is the
        /// starting point of the drag in world coordinates and the ray is the
        /// starting ray of the drag in world coordinates.
        SG::Hit hit;

        /// True if the drag is a grip drag.
        bool    is_grip_drag = false;

        /// True if currently in alternate input mode.
        bool    is_alternate_mode = false;

        /// Linear precision to use for the drag (if the widget uses it).
        float   linear_precision;

        /// Angular precision (in degrees) to use for the drag (if the widget
        /// uses it).
        float   angular_precision;
    };

    /// \name Dragging functions
    /// Derived classes must implement these for dragging.
    ///@{

    /// StartDrag() begins a drag operation with the given DragInfo. Derived
    /// classes should call this version before adding their own functions.
    virtual void StartDrag(const DragInfo &info) {
        start_info_   = info;
        path_to_this_ = info.hit.path.GetSubPath(*this);
    }

    /// Drag() continues a drag operation.
    virtual void ContinueDrag(const DragInfo &info) = 0;

    /// EndDrag() finishes the drag operation.
    virtual void EndDrag() = 0;

    /// This can be called during a drag to get the DragInfo used to start the
    /// current drag operation. It is undefined at other times.
    const DragInfo & GetStartDragInfo() const { return start_info_; }

    ///@}

  protected:
    /// \name Transformation Helper Functions
    /// These functions help convert between coordinate systems. They assume
    /// that DraggableWidget::StartDrag() has been called. Local coordinates
    /// are defined as the coordinates at the tail of the path (this
    /// widget). They can be converted to and from the coordinate system at the
    /// root of the path.
    ///@{

    /// Transforms a point from local coordinates.
    Point3f FromLocal(const Point3f &p) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.FromLocal(p);
    }

    /// Transforms a vector from local coordinates.
    Vector3f FromLocal(const Vector3f &v) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.FromLocal(v);
    }

    /// Transforms a point to local coordinates.
    Point3f ToLocal(const Point3f &p) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.ToLocal(p);
    }

    /// Transforms a vector to local coordinates.
    Vector3f ToLocal(const Vector3f &v) const {
        ASSERT(! path_to_this_.empty());
        return path_to_this_.ToLocal(v);
    }

    ///@}

  private:
    /// Saves the DragInfo at the start of a drag.
    DragInfo start_info_;

    /// Path to this widget saved in SavePathToThis;
    SG::NodePath path_to_this_;
};

typedef std::shared_ptr<DraggableWidget> DraggableWidgetPtr;
