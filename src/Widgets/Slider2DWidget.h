#pragma once

#include <memory>

#include "Widgets/SliderWidgetBase.h"

namespace Parser { class Registry; }

/// Slider2DWidget is a derived SliderWidgetBase that provides interaction
/// within a constrained rectangle in the XY plane.
class Slider2DWidget : public SliderWidgetBase<Vector2f> {
  public:
    virtual Vector2f GetInterpolated() const override;
    virtual Vector2f GetRayValue(const Ray &local_ray) override;
    virtual Vector2f GetGripValue(const Vector2f &start_value,
                                  const Point3f &p0,
                                  const Point3f &p1) override;
    virtual void UpdatePosition() override;

  protected:
    Slider2DWidget() {}

  private:
    /// Local X/Y coordinates at the start of a drag.
    Point2f start_coords_{ 0, 0 };

    friend class Parser::Registry;
};

typedef std::shared_ptr<Slider2DWidget> Slider2DWidgetPtr;
