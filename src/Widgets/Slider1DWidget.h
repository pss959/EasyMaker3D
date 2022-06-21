#pragma once

#include "Base/Memory.h"
#include "Widgets/SliderWidgetBase.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Slider1DWidget);

/// Slider1DWidget is a derived SliderWidgetBase that provides interaction
/// along a constrained linear path.
///
/// \ingroup Widgets
class Slider1DWidget : public SliderWidgetBase<float> {
  public:
    /// Sets the dimension used for the slider (0, 1, or 2). The default is 0.
    void SetDimension(int dim);

    /// Returns the dimension used for the slider (0, 1, or 2).
    int GetDimension() const { return dimension_; }

    virtual float GetInterpolated() const override;
    virtual float GetRayValue(const Ray &local_ray) override;
    virtual float GetGripMotion(const Point3f &p0, const Point3f &p1) override;

    virtual void  UpdatePosition() override;

  protected:
    Slider1DWidget() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int> dimension_{"dimension", 0};
    ///@}

    friend class Parser::Registry;
};
