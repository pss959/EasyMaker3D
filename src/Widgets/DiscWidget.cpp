#include "Widgets/DiscWidget.h"

#include <cmath>

#include <ion/math/angleutils.h>

#include "CoordConv.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"

void DiscWidget::AddFields() {
    Widget::AddFields();
    AddField(scaling_allowed_);
    AddField(scale_range_);
    AddField(plane_offset_);
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
    Vector3f scale = factor * GetScale();

    const Vector2f &range = GetScaleRange();
    for (int dim = 0; dim < 3; ++dim)
        scale[dim] = Clamp(scale[dim], range[0], range[1]);

    SetScale(scale);
    scale_changed_.Notify(*this, delta);
}

void DiscWidget::StartDrag(const DragInfo &info) {
    using ion::math::AngleBetween;
    using ion::math::Normalized;

    DraggableWidget::StartDrag(info);

    start_angle_ = GetRotationAngle_(GetRotation());
    start_scale_ = GetScale();

    if (info.is_grip) {
        start_orientation_ = info.grip_orientation;
        cur_action_ = Action_::kRotation;  // Only action for a grip drag.
    }
    else {
        // If the starting ray is close enough (within 10 degrees) to parallel
        // to the DiscWidget's plane, use edge-on rotation. Otherwise, figure
        // it out later after drag motion is detected.
        const Vector3f local_dir = WorldToWidget(info.ray.direction, true);
        const Anglef angle = AngleBetween(local_dir, Vector3f(0, 1, 0));
        cur_action_ = AreClose(angle.Degrees(), 90.f, 10.f) ?
            Action_::kEdgeOnRotation : Action_::kUnknown;

        if (cur_action_ == Action_::kEdgeOnRotation) {
            // For edge-on rotation, set the starting point to the intersection
            // point in local coordinates.
            start_point_ = WorldToWidget(info.hit.GetWorldPoint());
        }
        else {
            // For scale and regular rotation, intersect the starting ray with
            // the plane to get the starting point.
            end_point_ = start_point_ = GetRayPoint_(WorldToWidget(info.ray));
        }
    }

    SetActive(true);
}

void DiscWidget::ContinueDrag(const DragInfo &info) {
    using ion::math::AngleBetween;
    using ion::math::Normalized;

    if (info.is_grip) {
        ASSERT(cur_action_ == Action_::kRotation);
        UpdateRotation_(
            ComputeRotation_(start_orientation_, info.grip_orientation));
    }
    else {
        // Convert the ray into local coordinates.
        const Ray local_ray = WorldToWidget(info.ray);

        if (cur_action_ == Action_::kEdgeOnRotation) {
            UpdateRotation_(ComputeEdgeOnRotationAngle_(local_ray));
        }
        else {
            // Intersect the local ray with the plane to get the current point.
            end_point_ = GetRayPoint_(local_ray);

            // See if the action is now known.
            if (cur_action_ == Action_::kUnknown)
                cur_action_ = DetermineAction_(start_point_, end_point_);

            // Process the correct action.
            if (cur_action_ == Action_::kRotation) {
                UpdateRotation_(ComputeRotation_(start_point_, end_point_));
            }
            else if (cur_action_ == Action_::kScale) {
                UpdateScale_(start_point_, end_point_);
            }
        }
    }
}

void DiscWidget::EndDrag() {
    SetActive(false);
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
    using ion::math::Dot;
    using ion::math::LengthSquared;
    using ion::math::Normalized;

    // If the main direction of the motion is along a radius (as opposed to in
    // the direction of rotation), scale.
    const Vector3f motion_dir    = Normalized(p1 -   p0);
    const Vector3f dir_to_center = Normalized(Vector3f(p1));

    // If both scaling and rotation are allowed, bail if there isn't enough
    // motion to choose one.
    if (IsScalingAllowed() && LengthSquared(motion_dir) < .01f)
        return Action_::kUnknown;

    /// Min absolute dot product for motion vector to be a scale.
    const float kMinAbsScaleDot = .8f;
    return IsScalingAllowed() &&
        std::fabs(Dot(motion_dir, dir_to_center)) > kMinAbsScaleDot ?
        Action_::kScale : Action_::kRotation;
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
    RayCylinderIntersect(local_ray, radius, distance);
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

Anglef DiscWidget::ComputeRotation_(const Rotationf &rot0,
                                    const Rotationf &rot1) {
    // Project the change in rotation into the plane.
    const Rotationf rot = GetLocalPlane_().ProjectRotation(
        RotationDifference(rot0, rot1));

    // Return the angle from it.
    return GetRotationAngle_(rot);
}

void DiscWidget::UpdateRotation_(const Anglef &rot_angle) {
    // Update our rotation.
    SetRotation(Rotationf::FromAxisAndAngle(Vector3f::AxisY(),
                                            start_angle_ + rot_angle));

    // Invoke the callback.
    rotation_changed_.Notify(*this, rot_angle);
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
