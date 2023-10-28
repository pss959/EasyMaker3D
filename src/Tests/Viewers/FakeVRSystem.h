#pragma once

#include "Math/Types.h"
#include "SG/VRCamera.h"
#include "Util/Memory.h"
#include "Util/Tuning.h"
#include "Viewers/IVRSystem.h"

/// Fake implementation of IVRSystem interface.
///
/// \ingroup VR
class FakeVRSystem : public IVRSystem {
  public:
    size_t emit_count = 0;  ///< Counts calls to EmitEvents().

    FakeVRSystem() {}
    virtual ~FakeVRSystem() {}

    virtual bool Startup() override { return false; }
    virtual void Shutdown() override {}
    virtual bool LoadControllerModel(Hand hand,
                                     Controller::CustomModel &model) {
        return false;
    }
    virtual void SetControllers(const ControllerPtr &l_controller,
                                const ControllerPtr &r_controller) override {}
    virtual void InitRendering(IRenderer &renderer) override {}
    virtual void SetCamera(const SG::VRCameraPtr &cam) override {
        camera_ = cam;
    }
    virtual void Render(const SG::Scene &scene,
                        IRenderer &renderer) override {
        // Build a Frustum.
        Frustum frustum;
        frustum.SetSymmetricFOV(Anglef::FromDegrees(60), 1);
        frustum.pnear       = TK::kVRNearDistance;
        frustum.pfar        = TK::kVRFarDistance;
        frustum.viewport    = Viewport::BuildWithSize(Point2i(0, 0),
                                                      Vector2i(400, 400));
        frustum.position    = camera_ ?
            camera_->GetCurrentPosition() : Point3f::Zero();
        frustum.orientation = Rotationf::Identity();
        renderer.RenderScene(scene, frustum);
    }
    virtual void EmitEvents(std::vector<Event> &events) override {
        ++emit_count;
    }
    virtual bool IsHeadSetOn() const override { return false; }
  private:
    SG::VRCameraPtr camera_;
};

DECL_SHARED_PTR(FakeVRSystem);
