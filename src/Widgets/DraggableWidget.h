#pragma once

#include <memory>

#include "DragInfo.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "Util/Assert.h"
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
    /// \name Dragging functions
    /// Derived classes must implement these for dragging.
    ///@{

    /// StartDrag() begins a drag operation with the given DragInfo. Derived
    /// classes should call this version before adding their own functions.
    virtual void StartDrag(const DragInfo &info) {
        start_info_ = info;
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
    /// are defined as the coordinates at the DraggableWidget. They can be
    /// converted to and from the coordinate system at the root of the path.
    ///@{

    /// Transforms a point from local coordinates.
    Point3f FromLocal(const Point3f &p) const {
        ASSERT(! start_info_.path_to_widget.empty());
        return start_info_.path_to_widget.FromLocal(p);
    }

    /// Transforms a vector from local coordinates.
    Vector3f FromLocal(const Vector3f &v) const {
        ASSERT(! start_info_.path_to_widget.empty());
        return start_info_.path_to_widget.FromLocal(v);
    }

    /// Transforms a point to local coordinates.
    Point3f ToLocal(const Point3f &p) const {
        ASSERT(! start_info_.path_to_widget.empty());
        return start_info_.path_to_widget.ToLocal(p);
    }

    /// Transforms a vector to local coordinates.
    Vector3f ToLocal(const Vector3f &v) const {
        ASSERT(! start_info_.path_to_widget.empty());
        return start_info_.path_to_widget.ToLocal(v);
    }

    ///@}

  private:
    /// Saves the DragInfo at the start of a drag.
    DragInfo start_info_;
};

typedef std::shared_ptr<DraggableWidget> DraggableWidgetPtr;
