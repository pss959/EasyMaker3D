#pragma once

#include <memory>

#include "Widgets/SliderWidgetBase.h"

namespace Parser { class Registry; }

/// Slider2DWidget is a derived SliderWidgetBase that provides interaction
/// within a constrained rectangle in the XY plane.
class Slider2DWidget : public SliderWidgetBase<Vector2f> {
  public:
    virtual void AddFields() override;

    virtual Vector2f GetInterpolated() const override;
    virtual void PrepareForDrag() override;
    virtual Vector2f ComputeDragValue(const DragInfo &info,
                                      const Vector2f &start_value) override;
    virtual void UpdatePosition() override;

  protected:
    Slider2DWidget() {}

  private:
    /// Local X/Y coordinates at the start of a drag.
    Point2f start_coords_{ 0, 0 };

    /// Intersects a world coordinate ray with the XY plane to get a point.
    Point2f IntersectRay_(const Ray &ray);

    /// Grip-drag version of finding closest point on min/max segment of
    // sliding axis. Points are in local coordinates.
    Vector2f GetClosestValue_(const Vector2f &start_value,
                              const Point3f &start_point,
                              const Point3f &cur_point);

    friend class Parser::Registry;
};

typedef std::shared_ptr<Slider2DWidget> Slider2DWidgetPtr;
