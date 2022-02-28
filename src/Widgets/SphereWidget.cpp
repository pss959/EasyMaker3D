#include "Widgets/SphereWidget.h"

#include <ion/math/vectorutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"

void SphereWidget::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);

    start_rot_ = GetRotation();

    // Compute the radius of the bounds to use for the virtual sphere. Use the
    // point of the geometry bounds that is farthest from the origin for the
    // radius so that the sphere encloses the geometry at any rotation.
    Point3f corners[8];
    GetBounds().GetCorners(corners);
    float max_squared_dist = 0;
    for (int i = 0; i < 8; ++i) {
        const float dsq = ion::math::LengthSquared(Vector3f(corners[i]));
        max_squared_dist = std::max(max_squared_dist, dsq);
    }
    radius_ = std::sqrt(max_squared_dist);

    if (! info.is_grip) {
        // Intersect the virtual sphere, rather than using the intersection
        // point; otherwise, the local transformations applied to the
        // intersection point could affect the location.
        if (! GetIntersectionVector_(info.ray, start_vec_)) {
            ASSERTM(false, "No starting intersection on virtual sphere");
        }
        end_vec_ = start_vec_;
    }

    SetActive(true);
}

void SphereWidget::ContinueDrag(const DragInfo &info) {
    Rotationf rot;

    if (info.is_grip) {
        // Grip drag. Rotate using the change in ray directions.
        rot = Rotationf::RotateInto(GetStartDragInfo().ray.direction,
                                    info.ray.direction);
    }
    else {
        // Ray-based drag. Intersect the ray with the virtual sphere around the
        // Widget. If there is an intersection, get the unit vector from the
        // center to it. Otherwise, just use the previous end vector.
        Vector3f vec;
        if (GetIntersectionVector_(info.ray, vec))
            end_vec_ = vec;

        rot = Rotationf::RotateInto(start_vec_, end_vec_);
    }

    // Update the Widget rotation and notify observers.
    SetRotation(rot * start_rot_);
    rotation_changed_.Notify(*this, rot);
}

void SphereWidget::EndDrag() {
    SetActive(false);
}

bool SphereWidget::GetIntersectionVector_(const Ray &ray, Vector3f &vec) const {
    // Transform the ray into object coordinates, as the scale and translation
    // applied to the sphere affect the intersection.
    const Ray obj_ray =
        TransformRay(ray, GetCoordConv().GetRootToObjectMatrix());

    float distance;
    if (RaySphereIntersect(obj_ray, radius_, distance)) {
        vec = ion::math::Normalized(Vector3f(obj_ray.GetPoint(distance)));
        return true;
    }
    return false;
}

