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

    /// Redefines this to also store Controller info.
    virtual void SetSceneContext(const SceneContextPtr &context) override;

    /// Redefines this to update the Controller state.
    virtual void SetActive(bool is_active) override;

  protected:
    virtual Event::Device GetDevice() const override;
    virtual bool GetRay(const Event &event, Ray &ray) override;

    /// Redefines this to update the Controller state.
    virtual void ProcessCurrentHit(const SG::Hit &hit) override;

  private:
    ControllerPtr controller_;       ///< Controller to track.
    SG::NodePath  controller_path_;  ///< Scene path to Controller.
};
