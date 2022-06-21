#pragma once

#include <ion/math/vectorutils.h>

#include "App/DragInfo.h"
#include "Base/Defaults.h"
#include "Base/Memory.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Util/Assert.h"
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
    /// Sets a scale factor to apply to controller motion to scale for
    /// grip-based drags. The default value is Defaults::kGripDragScale.
    void SetGripDragScale(float scale) { grip_drag_scale_ = scale; }

    /// Returns the scale factor to apply to controller motion to scale for
    /// grip-based drags. The default value is Defaults::kGripDragScale.
    float GetGripDragScale() const { return grip_drag_scale_; }

    /// \name Dragging functions
    /// Derived classes must implement these for dragging.
    ///@{

    /// StartDrag() begins a drag operation with the given DragInfo. Derived
    /// classes should call this version before adding their own functions.
    virtual void StartDrag(const DragInfo &info) {
        start_info_ = cur_info_ = info;
    }

    /// Drag() continues a drag operation.
    virtual void ContinueDrag(const DragInfo &info) {
        cur_info_ = info;
    }

    /// EndDrag() finishes the drag operation.
    virtual void EndDrag() = 0;

    /// This can be called during a drag to get the DragInfo used to start the
    /// current drag operation. It is undefined at other times.
    const DragInfo & GetStartDragInfo() const { return start_info_; }

    /// This can be called during a drag to get the current DragInfo. It is
    /// undefined at other times.
    const DragInfo & GetCurrentDragInfo() const { return cur_info_; }

    ///@}

  protected:
    /// \name Coordinate Conversion Helpers
    /// Each of these functions uses the CoordConv instance in the DragInfo
    /// saved at the start of the drag. (They all assume that
    /// DraggableWidget::StartDrag() has been called.) They convert between the
    /// Widget's local coordinate system and other systems.
    ///@{

    /// Convenience that returns a CoordConv for the DragInfo at the start of
    /// the drag.
    CoordConv GetCoordConv() const {
        return CoordConv(start_info_.path_to_widget);
    }

    /// Convenience function that converts a point in the local coordinates of
    /// the Widget to world coordinates.
    Point3f WidgetToWorld(const Point3f &p) const {
        return GetCoordConv().LocalToRoot(p);
    }

    /// Convenience function that converts a vector in the local coordinates of
    /// the Widget to world coordinates.
    Vector3f WidgetToWorld(const Vector3f &v, bool normalize = false) const {
        const Vector3f nv = GetCoordConv().LocalToRoot(v);
        return normalize ? ion::math::Normalized(nv) : nv;
    }

    /// Convenience function that converts a point from world coordinates to
    /// the local coordinates of the Widget.
    Point3f WorldToWidget(const Point3f &p) const {
        return GetCoordConv().RootToLocal(p);
    }

    /// Convenience function that converts a vector from world coordinates to
    /// the local coordinates of the Widget.
    Vector3f WorldToWidget(const Vector3f &v, bool normalize = false) const {
        const Vector3f nv =  GetCoordConv().RootToLocal(v);
        return normalize ? ion::math::Normalized(nv) : nv;
    }

    /// Convenience function that converts a Ray from world coordinates to
    /// the local coordinates of the Widget.
    Ray WorldToWidget(const Ray &ray) const {
        return TransformRay(ray, GetCoordConv().GetRootToLocalMatrix());
    }

  private:
    /// Amount to scale controller motion for grip drags.
    float grip_drag_scale_ = Defaults::kGripDragScale;

    /// Saves the DragInfo at the start of a drag.
    DragInfo start_info_;
    /// Saves the DragInfo during a drag.
    DragInfo cur_info_;
};
