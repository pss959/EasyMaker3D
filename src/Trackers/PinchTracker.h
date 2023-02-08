#pragma once

#include "SG/NodePath.h"
#include "Trackers/PointerTracker.h"

DECL_SHARED_PTR(Controller);

/// PinchTracker is a derived PointerTracker class that tracks VR controller
/// pinches.
///
/// \ingroup Trackers
class PinchTracker : public PointerTracker {
  public:
    explicit PinchTracker(Actuator actuator);
    virtual Event::Device GetDevice() const override;

    virtual bool IsActivation(const Event &event, WidgetPtr &widget) override;
    virtual bool IsDeactivation(const Event &event, WidgetPtr &widget) override;
    virtual float GetClickTimeout() const override;

  protected:
    virtual bool GetRay(const Event &event, Ray &ray) override;
    virtual Anglef GetMinRayAngleChange() const override;

    /// Redefines this to update the Controller state.
    virtual void ProcessCurrentHit(const SG::Hit &hit) override;

    virtual bool IsLeft() const override {
        return GetActuator() == Actuator::kLeftPinch;
    }

  private:
    /// Updates the Controllers when the active state changes.
    void UpdateControllers_(bool is_active);
};
