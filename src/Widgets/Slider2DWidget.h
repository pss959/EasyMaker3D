#pragma once

#include <memory>

#include "Widgets/SliderWidgetBase.h"

namespace Parser { class Registry; }

/// Slider2DWidget is a derived SliderWidgetBase that provides interaction
/// within a constrained rectangle in the XY plane.
class Slider2DWidget : public SliderWidgetBase<Vector2f> {
  public:
    virtual Vector2f GetInterpolated() const override;
    virtual void PrepareForDrag(const DragInfo &info) override;
    virtual Vector2f ComputeDragValue(const DragInfo &info,
                                      const Vector2f &start_value) override;
    virtual void UpdatePosition() override;

  protected:
    Slider2DWidget() {}

  private:
    /// Local X/Y coordinates at the start of a drag.
    Point2f start_coords_{ 0, 0 };

    /// Ray version of computing a new value as the closest point on the XY
    /// plane in local coordinates.
    Vector2f GetRayValue_(const Ray &ray);

    /// Grip-drag version of finding a new value by using the relative change
    /// in controller positions (the two world-coordinate points).
    Vector2f GetGripValue_(const Vector2f &start_value,
                           const Point3f &p0, const Point3f &p1);

    friend class Parser::Registry;
};

typedef std::shared_ptr<Slider2DWidget> Slider2DWidgetPtr;
