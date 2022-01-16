#pragma once

#include <memory>

#include "Math/Types.h"
#include "Parser/Field.h"
#include "SG/NodePath.h"
#include "Widgets/DraggableWidget.h"

namespace Parser { class Registry; }

/// DiscWidget implements both rotation and scaling on an infinite disc
/// centered on the origin in a plane parallel to the XZ-plane (rotating about
/// the Y axis). The placement of the plane depends on the plane_offset field.
/// Note that scaling applies in all three dimensions.
/// \ingroup Widgets
class DiscWidget : public DraggableWidget {
  public:
    /// Returns a Notifier that is invoked when the user drags the widget to
    /// cause rotation. It is passed the widget and the change in rotation
    /// angle around the axis from the start of the drag.
    Util::Notifier<Widget&, const Anglef &> & GetRotationChanged() {
        return rotation_changed_;
    }

    /// Returns a Notifier that is invoked when the user drags the widget to
    /// cause a change in scale (when enabled). It is passed the widget and the
    /// change in scale as a signed value from the start of the drag.
    Util::Notifier<Widget&, float> & GetScaleChanged() {
        return scale_changed_;
    }

    /// Returns a flag indicating whether scaling is allowed.
    bool IsScalingAllowed() const { return scaling_allowed_; }

    /// Returns the scaling range, which is used only if scaling is allowed.
    /// Any scaling will be clamped to this range.
    const Vector2f & GetScaleRange() const { return scale_range_; }

    /// Applies a relative change to the scale of the DiscWidget.
    void ApplyScaleChange(float delta);

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    DiscWidget() {}

    virtual void AddFields() override;

  private:
    /// Action being performed during a drag.
    enum class Action_ {
        kRotation,        ///< Typical rotation.
        kEdgeOnRotation,  ///< Rotation with a ray close to the  widget's plane.
        kScale,           ///< Scaling.
        kUnknown          ///< Not determined yet.
    };

    // ------------------------------------------------------------------------
    // Variables.

    /// \name Parsed Fields
    ///@{
    Parser::TField<bool>     scaling_allowed_{"scaling_allowed", true};
    Parser::TField<Vector2f> scale_range_{"scale_range", {.01f, 1000.f}};
    Parser::TField<float>    plane_offset_{"plane_offset", 0};
    ///@}

    /// Current action being performed.
    Action_    cur_action_;

    /// Center of the disc in world coordinates.
    Point3f    world_center_;

    /// Plane containing the disc in world coordinates.
    Plane      world_plane_;

    /// Starting point of a pointer drag in world coordinates.
    Point3f    world_start_point_;
    /// Ending point of a pointer drag in world coordinates.
    Point3f    world_end_point_;

    /// Starting orientation for a grip drag.
    Rotationf  start_orientation_;

    /// Starting rotation.
    Rotationf  start_rot_;

    /// Starting scale.
    Vector3f   start_scale_;

    /// Notifies when the widget is rotated.
    Util::Notifier<Widget&, const Anglef &> rotation_changed_;

    /// Notifies when the widget is scaled.
    Util::Notifier<Widget&, float> scale_changed_;

    /// True if interactive scaling is allowed.
    bool is_scaling_allowed_ = true;

    /// For edge-on rotation, this scales the angle between the starting and
    /// current rays for reasonable effects.
    static constexpr float kEdgeOnRotationFactor_ = 4.f;

    // ------------------------------------------------------------------------
    // Functions.

    /// Ray version of computing the current world-space point.
    Point3f GetRayPoint_(const Ray &ray);

    /// Returns the Action_ to performed based on the motion between two
    // points. If there is not enough motion, it returns Action_::kUnknown.
    Action_ DetermineAction_(const Point3f &p0, const Point3f p1);

    /// Computes and returns a rotation based on start and end points in the
    // disc's plane.
    Rotationf ComputeRotation_(const Point3f &p0, const Point3f &p1);

    /// Computes and returns a rotation based on start and end orientations
    /// (for grip drags).
    Rotationf ComputeRotation_(const Rotationf &rot0, const Rotationf &rot1);

    /// Updates the transform based on the given rotation and notifies.
    void UpdateRotation_(const Rotationf &rot);

    /// Computes a new scale factor based on start end points, then updates the
    /// transform and notifies.
    void UpdateScale_(const Point3f &p0, const Point3f &p1);

    friend class Parser::Registry;
};

typedef std::shared_ptr<DiscWidget> DiscWidgetPtr;
