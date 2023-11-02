#include "Widgets/Slider2DWidget.h"

#include "Math/Intersection.h"
#include "Math/Linear.h"

void Slider2DWidget::AddFields() {
    AddField(principal_plane_.Init("principal_plane", PrincipalPlane::kXY));

    SliderWidgetBase::AddFields();
}

void Slider2DWidget::PrepareForDrag(const DragInfo &info) {
    // If this is a pointer-based drag, set up the intersection plane based on
    // the intersection point.
    if (info.trigger == Trigger::kPointer) {
        const int dim = GetDimension_();
        const float distance = WorldToWidget(info.hit.GetWorldPoint())[dim];
        intersection_plane_ = Plane(distance, GetAxis(dim));
    }
}

Vector2f Slider2DWidget::GetInterpolated() const {
    const auto &val = GetValue();
    const auto &min = GetMinValue();
    const auto &max = GetMaxValue();
    return Vector2f(Lerp(val[0], min[0], max[0]),
                    Lerp(val[1], min[1], max[1]));
}

void Slider2DWidget::UpdatePosition() {
    const Vector2f val = GetUnnormalizedValue();
    SetTranslation(ToVector3f_(val));
}

Vector2f Slider2DWidget::GetRayValue(const Ray &local_ray) {
    // Intersect the ray with the principal plane.
    float distance;
    if (RayPlaneIntersect(local_ray, intersection_plane_, distance))
        return Vector2f(ToPoint2f(local_ray.GetPoint(distance),
                                  GetDimension_()));
    else
        return Vector2f::Zero();  // Parallel to plane somehow.
}

Vector2f Slider2DWidget::GetControllerMotion(const Point3f &p0,
                                             const Point3f &p1) {
    // Construct a plane in world coordinates that passes through the first
    // point and is parallel to the plane converted to world coordinates.
    const int dim = GetDimension_();
    const Plane world_plane(p0, WidgetToWorld(GetAxis(dim)));

    // Project both points onto this plane and get the vector between them.
    const Vector3f vec =
        world_plane.ProjectPoint(p1) - world_plane.ProjectPoint(p0);

    // Convert this vector into widget coordinates and use the scaled values.
    return ToVector2f(WorldToWidget(vec), dim);
}

Vector3f Slider2DWidget::ToVector3f_(const Vector2f &v2) const {
    switch (GetPrincipalPlane()) {
      case PrincipalPlane::kXY:
        return Vector3f(v2[0], v2[1], 0);
      case PrincipalPlane::kXZ:
        return Vector3f(v2[0], 0, v2[1]);
      case PrincipalPlane::kYZ:
      default:
        return Vector3f(0, v2[0], v2[1]);
    }
}

int Slider2DWidget::GetDimension_() const {
    const auto pp = GetPrincipalPlane();
    return pp == PrincipalPlane::kXY ? 2 :
        pp == PrincipalPlane::kXZ ? 1 : 0;
}
