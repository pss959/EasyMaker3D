#pragma once

#include "Trackers/PointerTracker.h"

/// MouseTracker is a derived PointerTracker class that tracks the mouse.
///
/// \ingroup Trackers
class MouseTracker : public PointerTracker {
  public:
    explicit MouseTracker(Actuator actuator);
    virtual Event::Device GetDevice() const override;
    virtual bool IsActivation(const Event &event, WidgetPtr &widget) override;
    virtual bool IsDeactivation(const Event &event, WidgetPtr &widget) override;
    virtual float GetClickTimeout() const override;

  protected:
    virtual bool GetRay(const Event &event, Ray &ray) override;
    virtual Anglef GetMinRayAngleChange() const override;
    virtual void ProcessCurrentHit(const SG::Hit &hit) override;
};
