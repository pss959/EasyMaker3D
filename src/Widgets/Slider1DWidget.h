#pragma once

#include <memory>

#include "Widgets/SliderWidgetBase.h"

//! Slider1DWidget is a derived SliderWidgetBase that provides interaction
//! along a constrained linear path.
class Slider1DWidget : public SliderWidgetBase<float> {
  public:
    virtual void AddFields() override;

    //! Returns the dimension used for the slider (0, 1, or 2).
    int GetDimension() const { return dimension_; }

    virtual float GetInterpolated() const override;
    virtual void PrepareForDrag(const DragInfo &info,
                                const Point3f &start_point) override;
    virtual float ComputeDragValue(const DragInfo &info,
                                   const Point3f &start_point,
                                   const float &start_value,
                                   float precision) override;
    virtual void UpdatePosition() override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<int> dimension_{"dimension", 0};
    //!@}

    //! Coordinate in the sliding dimension at the start of a drag.
    float start_coord_ = 0;

    //! Ray version of finding closest point on min/max segment of sliding
    //! axis.
    float GetRayValue_(const Ray &ray);

    //! Grip-drag version of finding closest point on min/max segment of
    // sliding axis.
    float GetClosestValue_(float start_value, const Point3f &start_point,
                           const Point3f &cur_point);
};

typedef std::shared_ptr<Slider1DWidget> Slider1DWidgetPtr;
