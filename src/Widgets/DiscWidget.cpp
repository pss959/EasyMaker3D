#include "Widgets/DiscWidget.h"

#include <cmath>

#include <ion/math/angleutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Util/Tuning.h"

void DiscWidget::AddFields() {
    AddField(mode_.Init("mode",                 Mode::kRotateAndScale));
    AddField(scale_range_.Init("scale_range",
                               Vector2f(TK::kDiscWidgetMinScale,
                                        TK::kDiscWidgetMaxScale)));
    AddField(apply_to_widget_.Init("apply_to_widget", true));
    AddField(plane_offset_.Init("plane_offset", 0));

    Widget::AddFields();
}

bool DiscWidget::IsValid(std::string &details) {
    if (! Widget::IsValid(details))
        return false;

    // Any local rotation on the DiscWidget will be ignored and is likely an
    // error.
    if (GetRotation() != Rotationf::Identity()) {
        details = "Rotation will be ignored";
        return false;
    }

    return true;
}

void DiscWidget::ApplyScaleChange(float delta) {
    const float factor = 1.f + delta;
    if (apply_to_widget_) {
        Vector3f scale = factor * GetScale();
        const Vector2f &range = GetScaleRange();
        for (int dim = 0; dim < 3; ++dim)
            scale[dim] = Clamp(scale[dim], range[0], range[1]);

        SetScale(scale);
    }
    scale_changed_.Notify(*this, factor);
}

void DiscWidget::SetRotationAngle(const Anglef &angle) {
    ApplyRotationToWidget_(angle);
}

Anglef DiscWidget::GetRotationAngle() const {
    return GetRotationAngle_(GetRotation());
}

void DiscWidget::StartDrag(const DragInfo &info) {
    using ion::math::AngleBetween;
    using ion::math::Normalized;

    DraggableWidget::StartDrag(info);

    start_angle_ = GetRotationAngle();
    start_scale_ = GetScale();

    if (info.trigger == Trigger::kPointer) {
        if (GetMode() == Mode::kScaleOnly) {
            cur_action_ = Action_::kScale;
        }
        else {
            // Rotation is allowed. Check if the ray is close to parallel to
            // the DiscWidget's plane, resulting in edge-on rotation.
            if (IsAlmostEdgeOn_(info.ray))
                cur_action_ = Action_::kEdgeOnRotation;
            else if (GetMode() == Mode::kRotationOnly)
                cur_action_ = Action_::kRotation;
            else
                cur_action_ = Action_::kUnknown;  // Figure it out later.
        }

        // Set the start and end points based on the mode.  For edge-on
        // rotation, use the intersection point in local coordinates. For
        // regular rotation and for scale, intersect the ray with the plane.
        start_point_ = end_point_ = cur_action_ == Action_::kEdgeOnRotation ?
            WorldToWidget(info.hit.GetWorldPoint()) :
            GetRayPoint_(WorldToWidget(info.ray));
    }
    else {
        ASSERT(info.trigger == Trigger::kGrip);
        cur_action_ = Action_::kRotation;  // Only action for a grip drag.
    }
    SetActive(true);
}

void DiscWidget::ContinueDrag(const DragInfo &info) {
    using ion::math::AngleBetween;
    using ion::math::Normalized;

    DraggableWidget::ContinueDrag(info);

    if (info.trigger == Trigger::kPointer) {
        // Convert the ray into local coordinates.
        const Ray local_ray = WorldToWidget(info.ray);

        if (cur_action_ == Action_::kEdgeOnRotation) {
            const Anglef angle = ComputeEdgeOnRotationAngle_(local_ray);
            UpdateRotation_(angle);
        }
        else {
            // Intersect the local ray with the plane to get the current point.
            end_point_ = GetRayPoint_(local_ray);

            // See if the action is now known.
            if (cur_action_ == Action_::kUnknown)
                cur_action_ = DetermineAction_(start_point_, end_point_);

            // Process the correct action.
            if (cur_action_ == Action_::kRotation) {
                const Anglef angle = ComputeRotation_(start_point_, end_point_);
                UpdateRotation_(angle);
            }
            else if (cur_action_ == Action_::kScale) {
                UpdateScale_(start_point_, end_point_);
            }
        }
    }
    else {
        ASSERT(info.trigger == Trigger::kGrip);
        ASSERT(cur_action_ == Action_::kRotation);
        const Anglef angle = ComputeGripRotation_(
            GetStartDragInfo().grip_orientation,
            info.grip_orientation, info.grip_guide_direction);
        UpdateRotation_(angle);
    }
}

void DiscWidget::EndDrag() {
    SetActive(false);
    prev_rot_angle_ = start_angle_ = Anglef::FromDegrees(0);
}

Point3f DiscWidget::GetRayPoint_(const Ray &local_ray) {
    using ion::math::Distance;

    // Special case if the rotation is edge-on. In this case, just take a point
    // a reasonable distance along the ray and project it into the plane.
    if (cur_action_ == Action_::kEdgeOnRotation) {
        return GetLocalPlane_().ProjectPoint(
            local_ray.GetPoint(Distance(local_ray.origin, start_point_)));
    }
    else {
        float distance;
        if (RayPlaneIntersect(local_ray, GetLocalPlane_(), distance))
            return local_ray.GetPoint(distance);

        // If there was no intersection (almost impossible), just use the last
        // point so there is no new motion.
        else
            return end_point_;
    }
}

DiscWidget::Action_ DiscWidget::DetermineAction_(const Point3f &p0,
                                                 const Point3f p1) {
    using ion::math::AngleBetween;
    using ion::math::LengthSquared;
    using ion::math::Normalized;

    // This should not be called unless both scale and rotation are allowed.
    ASSERT(GetMode() == Mode::kRotateAndScale);

    // If the main direction of the motion is along a radius (as opposed to in
    // the direction of rotation), scale.
    const Vector3f motion    = Normalized(p1 -   p0);
    const Vector3f to_center = Normalized(Vector3f(p1));

    // Bail if there isn't enough motion to choose an action.
    if (LengthSquared(motion) < TK::kMinDiscWidgetMotion)
        return Action_::kUnknown;

    const bool is_scale =
        AngleBetween( motion, to_center) <= TK::kMaxDiscWidgetScaleAngle ||
        AngleBetween(-motion, to_center) <= TK::kMaxDiscWidgetScaleAngle;
    return is_scale ? Action_::kScale : Action_::kRotation;
}

bool DiscWidget::IsAlmostEdgeOn_(const Ray &ray) const {
    // See if the ray forms an angle of 10 degrees or less with the
    // DiscWidget's plane.
    const Vector3f local_dir = WorldToWidget(ray.direction, true);
    const Anglef angle = AngleBetween(local_dir, Vector3f(0, 1, 0));
    return AreClose(angle.Degrees(), 90.f, 10.f);
}

Anglef DiscWidget::ComputeEdgeOnRotationAngle_(const Ray &local_ray) {
    using ion::math::Distance;
    using ion::math::Normalized;

    // Edge-on rotation is handled specially. Intersecting the ray with
    // the plane for edge-on rotation is not stable or useful, so
    // intersect with a cylinder around the DiscWidget's axis (Y axis)
    // that has a radius based on the original intersection point.
    const float radius = Distance(start_point_, Point3f(0, start_point_[1], 0));
    float distance = 1;
    // If there is no intersection, leave the current rotation alone.
    if (! RayCylinderIntersect(local_ray, radius, distance))
        return RotationAngle(GetRotation());
    end_point_ = local_ray.GetPoint(distance);
    end_point_[1] = start_point_[1];  // Keep it in the same plane.

    // Compute the angle between the vectors from the rotation center
    // to the start and end points, and rotate proportional to that.
    const Vector3f start_vec = Normalized(Vector3f(start_point_));
    const Vector3f   cur_vec = Normalized(Vector3f(end_point_));
    return SignedAngleBetween_(start_vec, cur_vec);
}

Anglef DiscWidget::ComputeRotation_(const Point3f &p0, const Point3f &p1) {
    using ion::math::AngleBetween;
    using ion::math::Cross;
    using ion::math::Normalized;

    // Project both points onto the XZ plane and take the vector from the
    // origin to each result.
    const Vector3f vec0 = Normalized(Vector3f(p0[0], 0, p0[2]));
    const Vector3f vec1 = Normalized(Vector3f(p1[0], 0, p1[2]));

    // Return the signed angle between the vectors.
    return SignedAngleBetween_(vec0, vec1);
}

Anglef DiscWidget::ComputeGripRotation_(const Rotationf &rot0,
                                        const Rotationf &rot1,
                                        const Vector3f &guide_direction) {
    // Grip rotation is always around the grip guide, meaning it is a rotation
    // of the controller around its X axis. Determine the change in angle of
    // the controller around this axis (i.e., in the YZ plane).
    const Plane yz(0, Vector3f::AxisX());
    const Rotationf rot = yz.ProjectRotation(RotationDifference(rot0, rot1));
    Vector3f axis;
    Anglef   angle;
    rot.GetAxisAndAngle(&axis, &angle);

    // Negate if around the -X axis.
    if (axis[0] < 0)
        angle = -angle;

    // Also negate if the rotated guide direction is pointing opposite the Y
    // axis.
    const Vector3f rotated_dir = WorldToWidget(rot0 * guide_direction);
    if ((guide_direction[0] < 0) != (rotated_dir[1] < 0))
        angle = -angle;

    return angle;
}

void DiscWidget::UpdateRotation_(const Anglef &rot_angle) {
    using ion::math::AngleBetween;

    Anglef angle = rot_angle;

    // If the new angle is more than 180 degrees from the previous angle,
    // switch direction for consistency.
    const float prev_deg = prev_rot_angle_.Degrees();
    const float cur_deg  = rot_angle.Degrees();
    if (std::abs(cur_deg - prev_deg) > 180) {
        float new_deg = cur_deg;
        if (prev_deg < 0 && cur_deg > 0)
            new_deg -= 360;
        else if (prev_deg > 0 && cur_deg < 0)
            new_deg += 360;
        angle = Anglef::FromDegrees(new_deg);
    }

    ApplyRotationToWidget_(start_angle_ + angle);
    rotation_changed_.Notify(*this, angle);

    prev_rot_angle_ = angle;
}

void DiscWidget::ApplyRotationToWidget_(const Anglef &new_angle) {
    if (apply_to_widget_)
        SetRotation(Rotationf::FromAxisAndAngle(Vector3f::AxisY(), new_angle));
}

void DiscWidget::UpdateScale_(const Point3f &p0, const Point3f &p1) {
    using ion::math::Dot;
    using ion::math::Length;
    using ion::math::Normalized;

    // Get the vectors from the center to the endpoints.
    const Vector3f vec0 = Vector3f(p0);
    const Vector3f vec1 = Vector3f(p1);

    // If the vectors point in opposite directions, meaning that the current
    // point is on the opposite side of the center, do nothing.  Otherwise,
    // compute the relative scale difference using vector lengths.
    if (Dot(Normalized(vec0), Normalized(vec1)) > 0) {
        const float l0 = Length(vec0);
        const float l1 = Length(vec1);
        if (apply_to_widget_)
            SetScale(start_scale_);
        ApplyScaleChange((l1 - l0) / l0);
    }
}

Plane DiscWidget::GetLocalPlane_() const {
    return Plane(Point3f(0, plane_offset_, 0), Vector3f::AxisY());
}

Anglef DiscWidget::GetRotationAngle_(const Rotationf &rot) {
    Anglef   angle;
    Vector3f axis;
    rot.GetAxisAndAngle(&axis, &angle);

    // This must be a rotation around the +/- Y axis (or identity).
    ASSERTM(AreClose(angle.Radians(), 0.f) || AreClose(std::abs(axis[1]), 1.f),
            Util::ToString(rot));

    // Flip the angle if the axis is inverted.
    return axis[1] < 0 ? -angle : angle;
}

Anglef DiscWidget::SignedAngleBetween_(const Vector3f &v0, const Vector3f &v1) {
    // AngleBetween() always returns a positive angle.
    const Anglef angle = AngleBetween(v0, v1);

    // Use the orientation relative to the Y axis to get the sign.
    return Cross(v0, v1)[1] < 0 ? -angle : angle;
}
