#include "Widgets/SphereWidget.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"

void SphereWidget::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);

    start_rot_ = GetRotation();

    if (info.trigger == Trigger::kPointer) {
        // Get the center and radius of the virtual sphere in world coordinates.
        Point3f center;
        ComputeSphere_(center, radius_);

        // Set up the plane through the center and perpendicular to the ray.
        plane_ = Plane(center, -info.ray.direction);

        // Get the initial intersection point with the plane.
        float distance;
        RayPlaneIntersect(info.ray, plane_, distance);
        start_point_ = info.ray.GetPoint(distance);
    }

    SetActive(true);
}

void SphereWidget::ContinueDrag(const DragInfo &info) {
    DraggableWidget::ContinueDrag(info);

    Rotationf rot;
    if (info.trigger == Trigger::kPointer) {
        // Get the current intersection point with the plane.
        float distance;
        const Point3f cur_pt = RayPlaneIntersect(info.ray, plane_, distance) ?
            info.ray.GetPoint(distance) : start_point_;

        // Use the two points to create a rotation.
        const Vector3f diff = cur_pt - start_point_;
        const float length = ion::math::Length(diff);
        if (length < .0001f) {
            rot = Rotationf::Identity();
        }
        else {
            // The rotation axis is the vector in the plane that is
            // perpendicular to the difference vector.
            const Vector3f axis = ion::math::Cross(plane_.normal, diff);

            // Want a distance of 1 radius to result in a 90 degree rotation.
            const Anglef angle = Anglef::FromDegrees(90 * length / radius_);

            rot = Rotationf::FromAxisAndAngle(axis, angle);
        }
    }
    else {
        // Grip drag. Rotate using the change in orientations.
        ASSERT(info.trigger == Trigger::kGrip);
        const Rotationf &start_rot = GetStartDragInfo().grip_orientation;
        const Rotationf &diff_rot  = RotationDifference(start_rot,
                                                        info.grip_orientation);

        // Compute the true axis of rotation. This is the axis of the rotation
        // difference after the starting rotation is applied.
        const Vector3f axis = start_rot * RotationAxis(diff_rot);

        rot = Rotationf::FromAxisAndAngle(axis, RotationAngle(diff_rot));
    }

    // Update the Widget rotation and notify observers.
    SetRotation(ComposeRotations(start_rot_, rot));
    rotation_changed_.Notify(*this, rot);
}

void SphereWidget::EndDrag() {
    SetActive(false);
}

void SphereWidget::ComputeSphere_(Point3f &center, float &radius) const {
    using ion::math::DistanceSquared;

    // This is used to convert from the SphereWidget's object coordinates to
    // world coordinates.
    const Matrix4f owm = GetCoordConv().GetObjectToRootMatrix();

    // Convert the SphereWidget's bounds' center to world coordinates. This is
    // the center of the virtual sphere.
    const Bounds bounds = GetBounds();
    center = owm * bounds.GetCenter();

    // Get the 8 corners of the bounds in world coordinates.
    Point3f corners[8];
    bounds.GetCorners(corners);
    for (int i = 0; i < 8; ++i)
        corners[i] = owm * corners[i];

    // Find the farthest distance of any of the corners from the center in
    // world coordinates. Use this as the sphere radius.
    float r2 = 0;
    for (int i = 0; i < 8; ++i)
        r2 = std::max(r2, DistanceSquared(center, corners[i]));
    radius = std::sqrt(r2);
}
