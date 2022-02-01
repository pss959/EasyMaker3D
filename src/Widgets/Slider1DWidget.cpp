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

void Slider1DWidget::UpdatePosition() {
    SetTranslation(GetAxis(GetDimension(), GetUnnormalizedValue()));
}

float Slider1DWidget::GetRayValue(const Ray &local_ray) {
    // Find the closest point of the ray to the sliding axis.
    const int dim = GetDimension();
    const Point3f min_point = Point3f(GetAxis(dim, GetMinValue()));
    const Point3f max_point = Point3f(GetAxis(dim, GetMaxValue()));
    Point3f axis_pt, ray_pt;
    if (GetClosestLinePoints(min_point, max_point - min_point,
                               local_ray.origin, local_ray.direction,
                               axis_pt, ray_pt))
        return axis_pt[dim];
    else
        return 0;  // Parallel lines somehow.
}

float Slider1DWidget::GetGripValue(const float &start_value,
                                   const Point3f &p0, const Point3f &p1) {
    // Use the relative distance between the points in world coordinates along
    // the slider direction to compute the new value.
    const int dim = GetDimension();
    const Vector3f world_dir = WidgetToWorld(GetAxis(dim), true);

    // Compute the closest world-coordinate point to the second point on the
    // direction vector. The distance between p0 and this point is proportional
    // to the amount to change the slider value.
    const Point3f p = GetClosestPointOnLine(p1, p0, world_dir);
    return start_value + kGripDragScale * (p[dim] - p0[dim]);
}
