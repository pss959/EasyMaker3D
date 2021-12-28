#include "Widgets/Slider1DWidget.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

void Slider1DWidget::AddFields() {
    SliderWidgetBase<float>::AddFields();
    AddField(dimension_);
}

void Slider1DWidget::SetDimension(int dim) {
    ASSERT(dim >= 0 && dim <= 2);
    dimension_ = dim;
}

float Slider1DWidget::GetInterpolated() const {
    return Lerp(GetValue(), GetMinValue(), GetMaxValue());
}

float Slider1DWidget::ComputeDragValue(const DragInfo &info,
                                       const float &start_value) {
    // XXXX MOVE INTO BASE!!!!

    // For a grip drag, use the change in world coordinates along the slider
    // direction to get the base change in value. For a pointer drag, just
    // compute the new value as the closest position to the pointer ray.
    float val = info.is_grip ?
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

void Slider1DWidget::UpdatePosition() {
    SetTranslation(GetAxis(GetDimension(), GetUnnormalizedValue()));
}

float Slider1DWidget::GetRayValue_(const Ray &ray) {
    // Transform the ray into local coordinates.
    Ray local_ray(ToLocal(ray.origin), ToLocal(ray.direction));

    // Find the closest point of the ray to the sliding axis.
    const int dim = GetDimension();
    const Point3f min_point = Point3f(GetAxis(dim, GetMinValue()));
    const Point3f max_point = Point3f(GetAxis(dim, GetMaxValue()));
    Point3f axis_pt, ray_pt;
    if (! GetClosestLinePoints(min_point, max_point - min_point,
                               local_ray.origin, local_ray.direction,
                               axis_pt, ray_pt))
        axis_pt = min_point;  // Parallel lines somehow.
    return axis_pt[dim];
}

float Slider1DWidget::GetGripValue_(float start_value,
                                    const Point3f &p0, const Point3f &p1) {
    // Use the relative distance between the points in world coordinates along
    // the slider direction to compute the new value.
    const int dim = GetDimension();
    const Vector3f world_dir = FromLocal(GetAxis(dim), true);

    // Compute the closest world-coordinate point to the second point on the
    // direction vector. The distance between p0 and this point is proportional
    // to the amount to change the slider value.
    const Point3f p = GetClosestPointOnLine(p1, p0, world_dir);
    return start_value + kGripDragScale * (p[dim] - p0[dim]);
}
