#include "Widgets/Slider1DWidget.h"

#include <ion/math/transformutils.h>  // XXXX For matrix multiplication.

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

void Slider1DWidget::PrepareForDrag() {
    const int dim = GetDimension();
    ASSERT(dim >= 0 && dim <= 2);
    const auto &info = GetStartDragInfo();

    // Save to-local matrix. Use local because any scale, rotation, or
    // translation applied to the Widget itself should not affect interaction.
    start_matrix_ = info.path_to_widget.GetToLocalMatrix();

    // For a grip drag, save the local coordinates at the start of the drag.
    // For a pointer drag, intersect the ray to get the starting coordinates.
    if (info.is_grip)
        start_coord_ = info.GetLocalGripPosition()[dim];
    else
        start_coord_ = GetClosestRayValue_(info.ray);
}

float Slider1DWidget::ComputeDragValue(const DragInfo &info,
                                       const float &start_value) {
    float val = info.is_grip ?
        GetClosestValue_(start_value, GetStartDragInfo().grip_position,
                         info.grip_position) :
        GetClosestRayValue_(info.ray) - start_coord_;

    if (IsPrecisionBased() && info.linear_precision > 0.f)
        val = start_value + info.linear_precision * (val - start_value);
    else
        val += start_value;
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

float Slider1DWidget::GetClosestRayValue_(const Ray &ray) {
    const int dim = GetDimension();

    // Transform the ray into local coordinates assuming no translation.
    Ray local_ray(start_matrix_ * ray.origin, start_matrix_ * ray.direction);

    // Find the closest point of the ray to the sliding axis.
    const Point3f min_point = Point3f(GetAxis(dim, GetMinValue()));
    const Point3f max_point = Point3f(GetAxis(dim, GetMaxValue()));
    Point3f axis_pt, ray_pt;
    if (! GetClosestLinePoints(min_point, max_point - min_point,
                               local_ray.origin, local_ray.direction,
                               axis_pt, ray_pt))
        axis_pt = min_point;  // Parallel lines somehow.
    return axis_pt[dim];
}

float Slider1DWidget::GetClosestValue_(float start_value,
                                       const Point3f &start_point,
                                       const Point3f &cur_point) {
    // Use the change in grip position to determine the new value. Scale the
    // delta relative to the current precision so that finer control is easier.
    const float v0 = start_point[GetDimension()];
    const float v1 =   cur_point[GetDimension()];
    return start_value + kGripDragScale * (v1 - v0);
}
