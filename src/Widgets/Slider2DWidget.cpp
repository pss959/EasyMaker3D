#include "Widgets/Slider2DWidget.h"

#include <ion/math/vectorutils.h>

#include "Math/Intersection.h"
#include "Math/Linear.h"

/// Convenience to convert a Vector3f to a Vector2f in the X/Y plane.
static Vector2f ToVector2f(const Vector3f &v) {
    return ion::math::WithoutDimension(v, 2);
}

/// Convenience to convert a Point3f to a Point2f in the X/Y plane.
static Point2f ToPoint2f(const Point3f &v) {
    return ion::math::WithoutDimension(v, 2);
}

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

Vector2f Slider2DWidget::GetGripValue(const Vector2f &start_value,
                                      const Point3f &p0, const Point3f &p1) {
    // Construct a plane in world coordinates that passes through the first
    // point and is parallel to the XY-plane converted to world coordinates.
    const Plane world_plane(p0, FromLocal(GetAxis(2)));

    // Project the second point onto this plane and use the relative distance
    // between p0 and this to compute the new value.
    return start_value +
        kGripDragScale * ToVector2f(world_plane.ProjectPoint(p1) - p0);
}
