#include "Widgets/Slider1DWidget.h"

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/Enum.h"

void Slider1DWidget::AddFields() {
    AddField(dimension_.Init("dimension", Dim::kX));

    SliderWidgetBase<float>::AddFields();
}

void Slider1DWidget::SetDimension(Dim dim) {
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
    const Dim dim = GetDimension();
    const Point3f min_point = Point3f(GetAxis(dim, GetMinValue()));
    const Point3f max_point = Point3f(GetAxis(dim, GetMaxValue()));
    Point3f axis_pt, ray_pt;
    if (GetClosestLinePoints(min_point, max_point - min_point,
                             local_ray.origin, local_ray.direction,
                             axis_pt, ray_pt))
        return axis_pt[Util::EnumInt(dim)];
    else
        return 0;  // Parallel lines somehow.
}

float Slider1DWidget::GetControllerMotion(const Point3f &p0,
                                          const Point3f &p1) {
    // Transform the two points into object coordinates of the Widget and use
    // the change in the coordinate in the Widget's dimension.
    const int dim = Util::EnumInt(GetDimension());
    return WorldToWidget(p1)[dim] - WorldToWidget(p0)[dim];
}
