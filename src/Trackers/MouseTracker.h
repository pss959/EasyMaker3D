#pragma once

#include "Trackers/PointerTracker.h"

/// MouseTracker is a derived PointerTracker class that tracks the mouse.
///
/// \ingroup Trackers
class MouseTracker : public PointerTracker {
  public:
    explicit MouseTracker(Actuator actuator);

  protected:
    virtual Event::Device GetDevice() const override;
    virtual bool GetRay(const Event &event, Ray &ray) override;
};
