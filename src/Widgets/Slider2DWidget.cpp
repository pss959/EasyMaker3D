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

void Slider2DWidget::PrepareForDrag(const DragInfo &info,
                                    const Point3f &start_point) {
    start_coords_ = ToPoint2f(ToLocal(start_point));
    std::cerr << "XXXX start_point = " << start_point
              << " start_coords_ = " << start_coords_ << "\n";
}

Vector2f Slider2DWidget::ComputeDragValue(const DragInfo &info,
                                          const Point3f &start_point,
                                          const Vector2f &start_value,
                                          float precision) {
    // Zero out the current translation so it does not affect the transform.
    SetTranslation(Vector3f::Zero());

    Vector2f val = info.is_grip_drag ?
        GetClosestValue_(start_value, start_point, info.ray.origin) :
        GetRayValue_(info.ray);
    if (info.is_grip_drag)
        std::cerr << "XXXX Grip SV = " << start_value
                  << " SP = " << start_point
                  << " RO = " << info.ray.origin
                  << " VAL = " << val << "\n";
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

Vector2f Slider2DWidget::GetRayValue_(const Ray &ray) {
    // Transform the ray into local coordinates assuming no translation.
    Ray local_ray(ToLocal(ray.origin), ToLocal(ray.direction));

    // Intersect the ray with the XY plane.
    const Plane xy_plane;
    float distance;
    if (RayPlaneIntersect(local_ray, xy_plane, distance))
        return ToPoint2f(local_ray.GetPoint(distance)) - start_coords_;
    else
        return GetMinValue();  // Parallel to plane somehow.
}

Vector2f Slider2DWidget::GetClosestValue_(const Vector2f &start_value,
                                          const Point3f &start_point,
                                          const Point3f &cur_point) {
    // Use the change in local coordinates to determine the new value. Scale
    // the delta relative to the current precision so that finer control is
    // easier.
    const Point3f p0 = ToLocal(start_point);
    const Point3f p1 = ToLocal(cur_point);
    return start_value + kGripDragScale * ToVector2f(p1 - p0);
}
