#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Util/Notifier.h"
#include "Widgets/DraggableWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SphereWidget);

/// SphereWidget implements free rotation using a sphere centered on the
/// origin.  Note that the geometry used for the SphereWidget matters only for
/// computing the virtual sphere radius - it always uses the virtual sphere for
/// dragging.
///
/// \ingroup Widgets
class SphereWidget : public DraggableWidget {
  public:
    /// Returns a Notifier that is invoked when the user drags the widget to
    /// cause rotation. It is passed the widget and the rotation relative to
    /// the start of the drag in world coordinates.
    Util::Notifier<Widget &, const Rotationf &> & GetRotationChanged() {
        return rotation_changed_;
    }

    /// Returns the current rotation, even if in the middle of a drag.
    const Rotationf & GetCurrentRotation() const { return rot_; }

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    SphereWidget() {}

  private:
    /// Notifies when the widget is rotated.
    Util::Notifier<Widget&, const Rotationf &> rotation_changed_;

    /// Plane perpendicular to the initial ray in world coordinates. This is
    /// used to compute intersection points for pointer-based rotations.
    Plane plane_;

    /// Radius of the virtual sphere in world coordinates.
    float radius_;

    /// Current rotation of the Widget.
    Rotationf rot_;

    /// Rotation of the Widget at the start of the drag.
    Rotationf start_rot_;

    /// Initial intersection point with the plane.
    Point3f start_point_;

    /// Computes the center and radius of the virtual sphere and stores them in
    /// the given arguments.
    void ComputeSphere_(Point3f &center, float &radius) const;

    friend class Parser::Registry;
};
