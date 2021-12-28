#pragma once

#include <memory>

#include "Widgets/SliderWidgetBase.h"

namespace Parser { class Registry; }

/// Slider1DWidget is a derived SliderWidgetBase that provides interaction
/// along a constrained linear path.
class Slider1DWidget : public SliderWidgetBase<float> {
  public:
    virtual void AddFields() override;

    /// Sets the dimension used for the slider (0, 1, or 2). The default is 0.
    void SetDimension(int dim);

    /// Returns the dimension used for the slider (0, 1, or 2).
    int GetDimension() const { return dimension_; }

    virtual float GetInterpolated() const override;
    virtual float ComputeDragValue(const DragInfo &info,
                                   const float &start_value) override;
    virtual void UpdatePosition() override;

  protected:
    Slider1DWidget() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int> dimension_{"dimension", 0};
    ///@}

    /// Ray version of computing a new value as the closest point on the slider
    /// axis.
    float GetRayValue_(const Ray &ray);

    /// Grip-drag version of finding a new value by using the relative change
    /// in controller positions (the two world-coordinate points).
    float GetGripValue_(float start_value,
                        const Point3f &p0, const Point3f &p1);

    friend class Parser::Registry;
};

typedef std::shared_ptr<Slider1DWidget> Slider1DWidgetPtr;
