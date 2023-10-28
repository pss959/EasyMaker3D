#pragma once

#include <memory>

#include "Viewers/IVRSystem.h"

/// VRSystem is a derived IVRSystem that uses OpenVR for implementation.
///
/// \ingroup VR
class VRSystem : public IVRSystem {
  public:
    VRSystem();
    ~VRSystem();

    virtual bool Startup() override;
    virtual void Shutdown() override;
    virtual bool LoadControllerModel(Hand hand,
                                     Controller::CustomModel &model) override;
    virtual void SetControllers(const ControllerPtr &l_controller,
                                const ControllerPtr &r_controller) override;
    virtual void InitRendering(IRenderer &renderer) override;
    virtual void SetCamera(const SG::VRCameraPtr &cam) override;
    virtual void Render(const SG::Scene &scene, IRenderer &renderer) override;
    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual bool IsHeadSetOn() const override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};
