#include "Widgets/Slider2DWidget.h"

#include "Math/Intersection.h"
#include "Math/Linear.h"

Vector2f Slider2DWidget::GetInterpolated() const {
    return Lerp(GetValue(), GetMinValue(), GetMaxValue());
}

void Slider2DWidget::UpdatePosition() {
    const Vector2f val = GetUnnormalizedValue();
    SetTranslation(Vector3f(val[0], val[1], 0));
}

Vector2f Slider2DWidget::GetRayValue(const Ray &local_ray) {
    // Intersect the ray with the XY plane.
    const Plane xy_plane;
    float distance;
    if (RayPlaneIntersect(local_ray, xy_plane, distance))
        return Vector2f(ToPoint2f(local_ray.GetPoint(distance)));
    else
        return Vector2f::Zero();  // Parallel to plane somehow.
}

Vector2f Slider2DWidget::GetControllerMotion(const Point3f &p0,
                                             const Point3f &p1) {
    // Construct a plane in world coordinates that passes through the first
    // point and is parallel to the XY-plane converted to world coordinates.
    const Plane world_plane(p0, WidgetToWorld(GetAxis(2)));

    // Project both points onto this plane and get the vector between them.
    const Vector3f vec =
        world_plane.ProjectPoint(p1) - world_plane.ProjectPoint(p0);

    // Convert this vector into widget coordinates and use the scaled X and Y
    // values.
    return ToVector2f(WorldToWidget(vec));
}
