#pragma once

#include <memory>

#include "Math/Types.h"
#include "Util/Notifier.h"
#include "Widgets/DraggableWidget.h"

namespace Parser { class Registry; }

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
    /// the start of the drag.
    Util::Notifier<Widget&, const Rotationf &> & GetRotationChanged() {
        return rotation_changed_;
    }

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    SphereWidget() {}

  private:
    /// Notifies when the widget is rotated.
    Util::Notifier<Widget&, const Rotationf &> rotation_changed_;

    /// Radius used for the virtual sphere.
    float radius_ = 1;

    /// Rotation of the Widget at the start of the drag.
    Rotationf start_rot_;

    /// Unit vector from the center to the starting drag point.
    Vector3f start_vec_;

    /// Unit vector from the center to the current ending drag point. This is
    /// used when no new intersection is found.
    Vector3f end_vec_;

    // Intersects the given ray with the virtual sphere around the Widget. If
    // there is an intersection, this sets vec to the unit vector from the
    // origin to the intersection point. Otherwise, it just returns false.
    bool GetIntersectionVector_(const Ray &ray, Vector3f &vec) const;

    friend class Parser::Registry;
};

typedef std::shared_ptr<SphereWidget> SphereWidgetPtr;
