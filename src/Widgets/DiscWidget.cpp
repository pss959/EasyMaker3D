#include "DiscWidget.h"

#include <cmath>

#include <ion/math/angleutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"

void DiscWidget::StartDrag(const DragInfo &info) {
    start_rot_   = GetRotation();
    start_scale_ = GetScale();

    path_to_this_ = info.hit.path.GetSubPath(*this);
    world_center_ = path_to_this_.ToWorld(Point3f::Zero());
    world_plane_  = Plane(world_center_,
                          path_to_this_.ToWorld(Vector3f::AxisY()));
    world_center_ = world_plane_.ProjectPoint(world_center_);

    world_start_point_ = world_plane_.ProjectPoint(info.hit.point);
    world_end_point_   = world_start_point_;

    if (info.is_grip_drag)
        start_grip_ray_ = info.hit.world_ray;

    // If this is a pointer-based drag and the ray is close to parallel to the
    // DiscWidget's plane, use EdgeOnRotation.
    const bool is_parallel = ! info.is_grip_drag &&
        AreClose(ion::math::AngleBetween(info.hit.world_ray.direction,
                                         world_plane_.normal).Degrees(),
                 90.f, 1.f);

    if (is_parallel)
        cur_action_ = Action_::kEdgeOnRotation;
    else
        cur_action_ = Action_::kUnknown;

    SetActive(true);
}

void DiscWidget::Drag(const DragInfo &info) {
    Point3f cur_pt = info.is_grip_drag ?
        world_plane_.ProjectPoint(info.hit.world_ray.origin) :
        GetRayPoint_(info.hit.world_ray);

    // See if the action is now known.
    if (cur_action_ == Action_::kUnknown)
        cur_action_ = DetermineAction_(world_start_point_, cur_pt);

    // Process the correct action.
    if (cur_action_ == Action_::kEdgeOnRotation) {
        // Rotate proportional to the angle between the starting ray and the
        // current ray.
        Vector3f start_vec = world_center_ - info.hit.world_ray.origin;
        Vector3f   cur_vec = info.hit.world_ray.direction;
        const Anglef angle = kEdgeOnRotationFactor_ *
            -ion::math::AngleBetween(start_vec, cur_vec);
        UpdateRotation_(Rotationf::FromAxisAndAngle(world_plane_.normal,
                                                    angle));
    }
    else if (cur_action_ == Action_::kRotation) {
        Rotationf rot = info.is_grip_drag ?
            ComputeRotation_(start_grip_ray_, info.hit.world_ray) :
            ComputeRotation_(world_start_point_, cur_pt);
        UpdateRotation_(rot);
    }
    else if (cur_action_ == Action_::kScale)
        UpdateScale_(world_start_point_, cur_pt);

    world_end_point_ = cur_pt;
}

void DiscWidget::EndDrag() {
    SetActive(false);
}

Point3f DiscWidget::GetRayPoint_(const Ray &ray) {
    // Special case if the rotation is edge-on. In this case, just use a point
    // a reasonable distance along the ray projected into the plane.
    if (cur_action_ == Action_::kEdgeOnRotation) {
        return world_plane_.ProjectPoint(
            ray.GetPoint(ion::math::Distance(ray.origin, world_start_point_)));
    }
    else {
        float distance;
        if (RayPlaneIntersect(ray, world_plane_, distance))
            return ray.GetPoint(distance);

        // If there was no intersection (almost impossible), just use the last
        // point so there is no new motion.
        return world_end_point_;
    }
}

DiscWidget::Action_ DiscWidget::DetermineAction_(const Point3f &p0,
                                                 const Point3f p1) {
    using ion::math::Normalized;

    // If the main direction of the motion is along a radius (as opposed to in
    // the direction of rotation), scale.
    const Vector3f motion_dir    = Normalized(p1 -   p0);
    const Vector3f dir_to_center = Normalized(p1 - world_center_);

    std::cerr << "XXXX motion_dir    = " << motion_dir << "\n";
    std::cerr << "XXXX dir_to_center = " << dir_to_center << "\n";

    // If both scaling and rotation are allowed, bail if there isn't enough
    // motion to choose one.
    if (IsScalingAllowed() && ion::math::LengthSquared(motion_dir) < .01f)
        return Action_::kUnknown;

    std::cerr << "XXXX dot = " << ion::math::Dot(motion_dir, dir_to_center) << "\n";
    return IsScalingAllowed() &&
        std::fabs(ion::math::Dot(motion_dir, dir_to_center)) > .8f ?
        Action_::kScale : Action_::kRotation;
}

Rotationf DiscWidget::ComputeRotation_(const Point3f &p0, const Point3f &p1) {
    return Rotationf::RotateInto(p0 - world_center_, p1 - world_center_);
}

Rotationf DiscWidget::ComputeRotation_(const Ray &ray0, const Ray &ray1) {
    return world_plane_.ProjectRotation(
        Rotationf::RotateInto(ray0.direction, ray1.direction));
}

void DiscWidget::UpdateRotation_(const Rotationf &rot) {
    Anglef   angle;
    Vector3f axis;
    rot.GetAxisAndAngle(&axis, &angle);
    /* XXXX
    // Flip the angle if the axis is inverted.
    if (ion::math::Dot(transform.TransformVector(Vector3.up), axis) < 0f)
        angle = -angle;
    */

    // Update our rotation.
    SetRotation(start_rot_ * rot);

    // Invoke the callback.
    rotation_changed_.Notify(*this, angle);
}

void DiscWidget::UpdateScale_(const Point3f &p0, const Point3f &p1) {
    using ion::math::Dot;
    using ion::math::Length;
    using ion::math::Normalized;

    // Use the relative distances from the center to scale. Use local
    // coordinates so that the scale factors are appropriate.
    const Vector3f vec0 = p0 - world_center_;
    const Vector3f vec1 = p1 - world_center_;

    // If the vectors point in opposite directions, meaning that the current
    // point is on the opposite side of the center, do not change the scale.
    if (Dot(vec0, vec1) > 0) {
        // Compute the relative difference in terms of local coordinates. Use
        // the transformed unit vector along (1,1,1) to get a reasonable
        // distance to undo local-to-world scaling.
        const float local_dist = Length(
            path_to_this_.ToWorld(Normalized(Vector3f(1, 1, 1))));
        const float delta = (Length(vec1) - Length(vec0)) / local_dist;

        SetScale(ClampVector((delta + 1.f) * GetScale(),
                             kMinScale_, kMaxScale_));
        scale_changed_.Notify(*this, delta);
    }
}
