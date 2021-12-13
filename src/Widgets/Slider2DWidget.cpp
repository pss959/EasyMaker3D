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

void Slider2DWidget::AddFields() {
    SliderWidgetBase<Vector2f>::AddFields();
}

Vector2f Slider2DWidget::GetInterpolated() const {
    return Lerp(GetValue(), GetMinValue(), GetMaxValue());
}

void Slider2DWidget::PrepareForDrag() {
    // For a grip drag, save the local coordinates at the start of the drag.
    // For a pointer drag, intersect the ray to get the starting coordinates.
    const auto &info = GetStartDragInfo();
    if (info.is_grip)
        start_coords_ = ToPoint2f(info.local_point);
    else
        start_coords_ = IntersectRay_(info.ray);
}

Vector2f Slider2DWidget::ComputeDragValue(const DragInfo &info,
                                          const Vector2f &start_value,
                                          float precision) {
    // Zero out the current translation so it does not affect the transform.
    SetTranslation(Vector3f::Zero());

    Vector2f val = info.is_grip ?
        GetClosestValue_(start_value, GetStartDragInfo().world_point,
                         info.world_point) :
        IntersectRay_(info.ray) - start_coords_;
    if (precision > 0.f)
        val = start_value + precision * (val - start_value);
    val = Clamp(val, GetMinValue(), GetMaxValue());
    if (IsNormalized())
        val = (val - GetMinValue()) / (GetMaxValue() - GetMinValue());
    return val;
}

void Slider2DWidget::UpdatePosition() {
    const Vector2f val = GetUnnormalizedValue();
    SetTranslation(Vector3f(val[0], val[1], 0));
}

Point2f Slider2DWidget::IntersectRay_(const Ray &ray) {
    // Transform the ray into local coordinates assuming no translation.
    Ray local_ray(ToLocal(ray.origin), ToLocal(ray.direction));

    // Intersect the ray with the XY plane.
    const Plane xy_plane;
    float distance;
    if (RayPlaneIntersect(local_ray, xy_plane, distance))
        return ToPoint2f(local_ray.GetPoint(distance));
    else
        return Point2f::Zero();  // Parallel to plane somehow.
}

Vector2f Slider2DWidget::GetClosestValue_(const Vector2f &start_value,
                                          const Point3f &start_point,
                                          const Point3f &cur_point) {
    // Use the change in world coordinates to determine the new value. Scale
    // the delta relative to the current precision so that finer control is
    // easier.
    return start_value + kGripDragScale * ToVector2f(cur_point - start_point);
}
