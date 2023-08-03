#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Parser/Field.h"
#include "SG/NodePath.h"
#include "Widgets/DraggableWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(DiscWidget);

/// DiscWidget implements both rotation and scaling on an infinite disc
/// centered on the origin in a plane parallel to the XZ-plane (rotating about
/// the Y axis) in the DiscWidget's local coordinates. The placement of the
/// plane depends on the plane_offset field. Scaling applies in all three
/// dimensions.
///
/// Note that any rotation applied directly to the DiscWidget will be replaced
/// during the first interaction. To change the orientation, rotate any
/// ancestor of the DiscWidget or use SetRotationAngle().
///
/// \ingroup Widgets
class DiscWidget : public DraggableWidget {
  public:
    /// Defines modes of operation.
    enum class Mode {
        kRotationOnly,
        kScaleOnly,
        kRotateAndScale,
    };

    /// Defines modes used to compute the angle during rotation.
    enum class AngleMode {
        /// Clamp the current angle to the range [-360, 360] (default).
        kClamp,
        /// Accumulate the change in angle continuously.
        kAccumulate,
    };

    /// Returns a Notifier that is invoked when the user drags the widget to
    /// cause rotation. It is passed the widget and the change in rotation
    /// angle around the axis from the start of the drag.
    Util::Notifier<Widget&, const Anglef &> & GetRotationChanged() {
        return rotation_changed_;
    }

    /// Returns a Notifier that is invoked when the user drags the widget to
    /// cause a change in scale (when enabled). It is passed the widget and the
    /// new scale resulting from the drag, assuming the scale is 1 initially.
    Util::Notifier<Widget&, float> & GetScaleChanged() {
        return scale_changed_;
    }

    /// Returns the mode of operation. The default is Mode::kRotateAndScale.
    Mode GetMode() const { return mode_; }

    /// Returns the mode used to compute the angle during rotation. The default
    /// is AngleMode::kClamp.
    AngleMode GetAngleMode() const { return angle_mode_; }

    /// Returns the scaling range, which is used only if scaling is allowed.
    /// Any scaling will be clamped to this range.
    const Vector2f & GetScaleRange() const { return scale_range_; }

    /// Applies a relative change to the scale of the DiscWidget.
    virtual void ApplyScaleChange(float delta);

    /// Sets the rotation angle. This can be called at any time, including
    /// during a rotation drag to modify the rotation (to apply precision or
    /// snapping, for example). This does not invoke callbacks.
    void SetRotationAngle(const Anglef &angle);

    /// Returns the current rotation angle. This can be called at any time,
    /// including during a rotation drag.
    Anglef GetRotationAngle() const { return cur_angle_; }

    /// Returns the current scale factor. This can be called at any time,
    /// including during a scale drag.
    float GetScaleFactor() const { return scale_factor_; }

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    DiscWidget() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

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
    Parser::EnumField<Mode>      mode_;
    Parser::EnumField<AngleMode> angle_mode_;
    Parser::TField<Vector2f>     scale_range_;
    Parser::TField<bool>         apply_to_widget_;
    Parser::TField<float>        plane_offset_;
    ///@}

    /// Current action being performed.
    Action_    cur_action_;

    /// Starting point of a pointer drag in local coordinates. For edge-on
    /// rotation, this is the intersection point with the local geometry. For
    /// other actions, this is the intersection of the ray with the
    /// DiscWidget's plane.
    Point3f    start_point_;

    /// Ending point of a pointer drag in local coordinates.
    Point3f    end_point_;

    /// Starting angle.
    Anglef     start_angle_;

    /// Starting scale.
    Vector3f   start_scale_;

    /// Change in rotation angle from the previous drag.
    Anglef     prev_rot_angle_;

    /// Current rotation angle, taking the AngleMode into account.
    Anglef     cur_angle_;

    /// Current scale factor.
    float      scale_factor_ = 1;

    /// Notifies when the widget is rotated.
    Util::Notifier<Widget&, const Anglef &> rotation_changed_;

    /// Notifies when the widget is scaled.
    Util::Notifier<Widget&, float>          scale_changed_;

    /// True if interactive scaling is allowed.
    bool is_scaling_allowed_ = true;

    // ------------------------------------------------------------------------
    // Functions.

    /// Finds a point on the plane of the DiscWidget in local coordinates
    /// based on the current action and the given ray (local coordinates).
    Point3f GetRayPoint_(const Ray &local_ray);

    /// Returns the Action_ to performed based on the motion between two points
    /// (in local coordinates). If there is not enough motion, it returns
    /// Action_::kUnknown.
    Action_ DetermineAction_(const Point3f &p0, const Point3f p1);

    /// Returns true if the given ray is close enough to parallel to the
    /// DiscWidget's plane to use edge-on rotation.
    bool IsAlmostEdgeOn_(const Ray &ray) const;

    /// Computes and returns the angle to use for edge-on rotation for pointer
    /// drags.
    Anglef ComputeEdgeOnRotationAngle_(const Ray &local_ray);

    /// Computes and returns a rotation angle based on start and end points in
    // the disc's plane.
    Anglef ComputeRotation_(const Point3f &p0, const Point3f &p1);

    /// Computes and returns a rotation angle based on start and end
    /// orientations (for grip drags).
    Anglef ComputeGripRotation_(const Rotationf &rot0, const Rotationf &rot1,
                                const Vector3f &guide_direction);

    /// Updates the rotation based on the given change in rotation angle and
    /// notifies.
    void UpdateRotation_(const Anglef &rot_angle);

    /// Sets the rotation in the widget to the given angle if apply_to_widget_
    /// is true.
    void ApplyRotationToWidget_(const Anglef &new_angle);

    /// Computes a new scale factor based on start end points, then updates the
    /// scale and notifies.
    void UpdateScale_(const Point3f &p0, const Point3f &p1);

    /// Returns the plane of the DiscWidget in local coordinates.
    Plane GetLocalPlane_() const;

    /// Returns the signed angle between two vectors.
    static Anglef SignedAngleBetween_(const Vector3f &v0, const Vector3f &v1);

    friend class Parser::Registry;
};
