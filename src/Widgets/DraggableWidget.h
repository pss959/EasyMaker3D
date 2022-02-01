#pragma once

#include <memory>

#include <ion/math/vectorutils.h>

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
    /// \name Coordinate Conversion Helpers
    /// Each of these functions uses the CoordConv instance in the DragInfo
    /// saved at the start of the drag. (They all assume that
    /// DraggableWidget::StartDrag() has been called.) They convert between the
    /// Widget's local coordinate system and other systems.
    ///@{

    /// Convenience that returns the CoordConv in the DragInfo at the start of
    /// the drag.
    const CoordConv & GetCoordConv() const {
        ASSERT(! start_info_.path_to_widget.empty());
        return start_info_.coord_conv;
    }

    /// Convenience function that converts a point in the local coordinates of
    /// the Widget to world coordinates.
    Point3f WidgetToWorld(const Point3f &p) const {
        const auto &path = start_info_.path_to_widget;
        ASSERT(! path.empty());
        return start_info_.coord_conv.LocalToWorld(path, p);
    }

    /// Convenience function that converts a vector in the local coordinates of
    /// the Widget to world coordinates.
    Vector3f WidgetToWorld(const Vector3f &v, bool normalize = false) const {
        const auto &path = start_info_.path_to_widget;
        ASSERT(! path.empty());
        const Vector3f nv =  start_info_.coord_conv.LocalToWorld(path, v);
        return normalize ? ion::math::Normalized(nv) : nv;
    }

    /// Convenience function that converts a point from world coordinates to
    /// the local coordinates of the Widget.
    Point3f WorldToWidget(const Point3f &p) const {
        const auto &path = start_info_.path_to_widget;
        ASSERT(! path.empty());
        return start_info_.coord_conv.WorldToLocal(path, p);
    }

    /// Convenience function that converts a vector from world coordinates to
    /// the local coordinates of the Widget.
    Vector3f WorldToWidget(const Vector3f &v, bool normalize = false) const {
        const auto &path = start_info_.path_to_widget;
        ASSERT(! path.empty());
        const Vector3f nv =  start_info_.coord_conv.WorldToLocal(path, v);
        return normalize ? ion::math::Normalized(nv) : nv;
    }

  private:
    /// Saves the DragInfo at the start of a drag.
    DragInfo start_info_;
};

typedef std::shared_ptr<DraggableWidget> DraggableWidgetPtr;
