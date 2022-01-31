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
    /// saved at the start of the drag. They all assume that
    /// DraggableWidget::StartDrag() has been called.
    ///@{

    /// Convenience that returns the CoordConv in the DragInfo at the start of
    /// the drag.
    const CoordConv & GetCoordConv() const {
        ASSERT(! start_info_.path_to_widget.empty());
        return start_info_.coord_conv;
    }

    /// Convenience function that uses the CoordConv returned by GetCoordConv()
    /// to convert a point in the object coordinates of the object in the Hit
    /// to world coordinates.
    Point3f HitObjectToWorld(const Point3f &p) const {
        return GetCoordConv().ObjectToWorld(start_info_.hit.path, p);
    }

    /// Convenience function that converts a vector in the object coordinates
    /// of the object in the Hit to world coordinates.
    Vector3f HitObjectToWorld(const Vector3f &v, bool normalize = false) const {
        const Vector3f nv =
            GetCoordConv().ObjectToWorld(start_info_.hit.path, v);
        return normalize ? ion::math::Normalized(nv) : nv;
    }

    /// Convenience function that uses the CoordConv returned by GetCoordConv()
    /// to convert a point in the local coordinates of the object in the Hit to
    /// world coordinates.
    Point3f HitLocalToWorld(const Point3f &p) const {
        return GetCoordConv().LocalToWorld(start_info_.hit.path, p);
    }

    /// Convenience function that converts a vector in the local coordinates
    /// of the object in the Hit to world coordinates.
    Vector3f HitLocalToWorld(const Vector3f &v, bool normalize = false) const {
        const Vector3f nv =
            GetCoordConv().LocalToWorld(start_info_.hit.path, v);
        return normalize ? ion::math::Normalized(nv) : nv;
    }

    /// Convenience function that uses the CoordConv returned by GetCoordConv()
    /// to convert a point from world coordinates to the local coordinates of
    /// the object in the Hit.
    Point3f HitWorldToLocal(const Point3f &p) const {
        return GetCoordConv().WorldToLocal(start_info_.hit.path, p);
    }

    /// Convenience function that uses the CoordConv returned by GetCoordConv()
    /// to convert a vector from world coordinates to the local coordinates of
    /// the object in the Hit.
    Vector3f HitWorldToLocal(const Vector3f &v, bool normalize = false) const {
        const Vector3f nv =
            GetCoordConv().WorldToLocal(start_info_.hit.path, v);
        return normalize ? ion::math::Normalized(nv) : nv;
    }

  private:
    /// Saves the DragInfo at the start of a drag.
    DragInfo start_info_;
};

typedef std::shared_ptr<DraggableWidget> DraggableWidgetPtr;
