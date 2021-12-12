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

void Slider1DWidget::PrepareForDrag(const DragInfo &info,
                                    const Point3f &start_point) {
    ASSERT(GetDimension() >= 0 && GetDimension() <= 2);
    start_coord_ = ToLocal(start_point)[GetDimension()];
}

float Slider1DWidget::ComputeDragValue(const DragInfo &info,
                                       const Point3f &start_point,
                                       const float &start_value,
                                       float precision) {
    // Zero out the current translation so it does not affect the transform.
    SetTranslation(Vector3f::Zero());

    float val = info.is_grip_drag ?
        GetClosestValue_(start_value, start_point, info.ray.origin) :
        GetRayValue_(info.ray);
    if (precision > 0.f)
        val = start_value + precision * (val - start_value);
    val = Clamp(val, GetMinValue(), GetMaxValue());
    if (IsNormalized())
        val = (val - GetMinValue()) / (GetMaxValue() - GetMinValue());
    return val;
}

void Slider1DWidget::UpdatePosition() {
    Vector3f pos(0, 0, 0);
    pos[GetDimension()] = GetUnnormalizedValue();
    SetTranslation(pos);
}

float Slider1DWidget::GetRayValue_(const Ray &ray) {
    const int dim = GetDimension();

    // Transform the ray into local coordinates assuming no translation.
    Ray local_ray(ToLocal(ray.origin), ToLocal(ray.direction));

    // Find the closest point of the ray to the sliding axis.
    const Point3f min_point = Point3f(GetAxis(dim, GetMinValue()));
    const Point3f max_point = Point3f(GetAxis(dim, GetMaxValue()));
    Point3f axis_pt, ray_pt;
    if (! GetClosestLinePoints(min_point, max_point - min_point,
                               local_ray.origin, local_ray.direction,
                               axis_pt, ray_pt))
        axis_pt = min_point;  // Parallel lines somehow.

    return axis_pt[dim] - start_coord_;
}

float Slider1DWidget::GetClosestValue_(float start_value,
                                       const Point3f &start_point,
                                       const Point3f &cur_point) {
    // Use the change in local coordinates to determine the new value. Scale
    // the delta relative to the current precision so that finer control is
    // easier.
    const float v0 = ToLocal(start_point)[GetDimension()];
    const float v1 = ToLocal(cur_point)[GetDimension()];
    return start_value + kGripDragScale * (v1 - v0);
}
