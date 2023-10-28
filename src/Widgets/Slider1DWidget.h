#pragma once

#include "Enums/Dim.h"
#include "Util/Memory.h"
#include "Widgets/SliderWidgetBase.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Slider1DWidget);

/// Slider1DWidget is a derived SliderWidgetBase that provides interaction
/// along a constrained linear path.
///
/// \ingroup Widgets
class Slider1DWidget : public SliderWidgetBase<float> {
  public:
    /// Sets the dimension used for the slider. The default is Dim::kX.
    void SetDimension(Dim dim);

    /// Returns the dimension used for the slider.
    Dim GetDimension() const { return dimension_; }

    virtual float GetInterpolated() const override;
    virtual float GetRayValue(const Ray &local_ray) override;
    virtual float GetControllerMotion(const Point3f &p0,
                                      const Point3f &p1) override;

    virtual void  UpdatePosition() override;

  protected:
    Slider1DWidget() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Dim> dimension_;
    ///@}

    friend class Parser::Registry;
};
