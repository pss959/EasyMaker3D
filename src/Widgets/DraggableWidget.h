#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Place/DragInfo.h"
#include "Widgets/ClickableWidget.h"

DECL_SHARED_PTR(DraggableWidget);

/// DraggableWidget is an abstract base class for widgets that implement
/// interactive dragging. There are two modes of dragging: ray-based and
/// grip-based.  Ray-based drags are performed with the mouse or with
/// laser-pointer/pinch of a VR controller. Grip-based drags are always done
/// with a controller.
///
/// DraggableWidget is derived from ClickableWidget, since draggable widgets
/// should also implement clicking and/or alt-clicking to reset.
///
/// \ingroup Widgets
class DraggableWidget : public ClickableWidget {
  public:
    /// Returns a Notifier that is invoked when an Event causes the
    /// DraggableWidget to become or remain hovered. The hovered point on the
    /// Widget is passed in.
    Util::Notifier<const Point3f &> & GetHovered() { return hovered_; }

    /// Returns a Notifier that is invoked when the user drags the
    /// DraggableWidget. It is passed a pointer to DragInfo for the drag, and a
    /// flag indicating whether this is the start of the drag. The end of the
    /// drag is indicated by a null DragInfo pointer.
    Util::Notifier<const DragInfo *, bool> & GetDragged() { return dragged_; }

    /// Sets a scale factor to apply to controller motion to scale for
    /// grip-based drags. The default value is TK::kGripDragScale.
    void SetGripDragScale(float scale) { grip_drag_scale_ = scale; }

    /// Returns the scale factor to apply to controller motion to scale for
    /// grip-based drags. The default value is TK::kGripDragScale.
    float GetGripDragScale() const { return grip_drag_scale_; }

    /// \name Dragging functions
    /// Derived classes must implement these for dragging.
    ///@{

    /// StartDrag() begins a drag operation with the given DragInfo. Derived
    /// classes should call this version before adding their own functions.
    virtual void StartDrag(const DragInfo &info);

    /// Drag() continues a drag operation.
    virtual void ContinueDrag(const DragInfo &info);

    /// EndDrag() finishes the drag operation. Derived classes should call this
    /// version before adding their own functions.
    virtual void EndDrag();

    /// This can be called during a drag to get the DragInfo used to start the
    /// current drag operation. It is undefined at other times.
    const DragInfo & GetStartDragInfo() const { return start_info_; }

    /// This can be called during a drag to get the current DragInfo. It is
    /// undefined at other times.
    const DragInfo & GetCurrentDragInfo() const { return cur_info_; }

    /// Returns true if the DraggableWidget is in the middle of a drag
    /// operation.
    bool IsDragging() const { return is_dragging_; }

    ///@}

    /// Redefines this to invoke the GetHovered() Notifier.
    virtual void UpdateHoverPoint(const Point3f &point) override;

  protected:
    DraggableWidget();

    /// \name Coordinate Conversion Helpers
    /// Each of these functions uses the SG::CoordConv instance in the DragInfo
    /// saved at the start of the drag. (They all assume that
    /// DraggableWidget::StartDrag() has been called.) They convert between the
    /// Widget's local coordinate system and other systems.
    ///@{

    /// Convenience that returns an SG::CoordConv for the DragInfo at the start
    /// of the drag.
    SG::CoordConv GetCoordConv() const;

    /// Convenience function that converts a vector in the local coordinates of
    /// the Widget to world coordinates.
    Vector3f WidgetToWorld(const Vector3f &v, bool normalize = false) const;

    /// Convenience function that converts a point from world coordinates to
    /// the local coordinates of the Widget.
    Point3f WorldToWidget(const Point3f &p) const;

    /// Convenience function that converts a vector from world coordinates to
    /// the local coordinates of the Widget.
    Vector3f WorldToWidget(const Vector3f &v, bool normalize = false) const;

    /// Convenience function that converts a Ray from world coordinates to
    /// the local coordinates of the Widget.
    Ray WorldToWidget(const Ray &ray) const;

  private:
    Util::Notifier<const Point3f &>        hovered_;
    Util::Notifier<const DragInfo *, bool> dragged_;

    /// Set to true during a drag.
    bool is_dragging_ = false;

    /// Amount to scale controller motion for grip drags.
    float grip_drag_scale_;

    /// Saves the DragInfo at the start of a drag.
    DragInfo start_info_;
    /// Saves the DragInfo during a drag.
    DragInfo cur_info_;
};
