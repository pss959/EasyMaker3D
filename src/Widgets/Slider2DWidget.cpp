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

void Slider2DWidget::PrepareForDrag(const DragInfo &info) {
#if XXXX
    // For a grip drag, save the local coordinates at the start of the drag.
    // For a pointer drag, intersect the ray to get the starting coordinates.
    if (info.is_grip)
        start_coords_ = ToPoint2f(info.GetLocalGripPosition());
    else
        start_coords_ = IntersectRay_(info.ray);
#endif
}

Vector2f Slider2DWidget::ComputeDragValue(const DragInfo &info,
                                          const Vector2f &start_value) {
    // For a grip drag, use the change in world coordinates along the slider
    // direction to get the base change in value. For a pointer drag, just
    // compute the new value as the closest position to the pointer ray.
    Vector2f val = info.is_grip ?
        GetGripValue_(start_value,
                      GetStartDragInfo().grip_position, info.grip_position) :
        GetRayValue_(info.ray);

    // If this is precision-based, use the precision value to scale the change
    // in value.
    if (IsPrecisionBased() && info.linear_precision > 0)
        val = start_value + info.linear_precision * (val - start_value);
    val = Clamp(val, GetMinValue(), GetMaxValue());

    if (IsNormalized())
        val = (val - GetMinValue()) / (GetMaxValue() - GetMinValue());
    return val;
}

void Slider2DWidget::UpdatePosition() {
    const Vector2f val = GetUnnormalizedValue();
    SetTranslation(Vector3f(val[0], val[1], 0));
}

Vector2f Slider2DWidget::GetRayValue_(const Ray &ray) {
    // Transform the ray into local coordinates assuming no translation.
    Ray local_ray(ToLocal(ray.origin), ToLocal(ray.direction));

    // Intersect the ray with the XY plane.
    const Plane xy_plane;
    float distance;
    if (RayPlaneIntersect(local_ray, xy_plane, distance))
        return Vector2f(ToPoint2f(local_ray.GetPoint(distance)));
    else
        return Vector2f::Zero();  // Parallel to plane somehow.
}

Vector2f Slider2DWidget::GetGripValue_(const Vector2f &start_value,
                                       const Point3f &p0, const Point3f &p1) {
    // Construct a plane in world coordinates that passes through the first
    // point and is parallel to the XY-plane converted to world coordinates.
    const Plane world_plane(p0, FromLocal(GetAxis(2)));

    // Project the second point onto this plane and use the relative distance
    // between p0 and this to compute the new value.
    return start_value +
        kGripDragScale * ToVector2f(world_plane.ProjectPoint(p1) - p0);
}
