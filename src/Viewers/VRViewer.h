#pragma once

#include "Base/Memory.h"
#include "Viewers/Viewer.h"

class VRContext;

DECL_SHARED_PTR(VRViewer);
namespace SG { DECL_SHARED_PTR(VRCamera); }

/// VRViewer is a derived Viewer that uses OpenXR to view in VR and produce
/// events from VR devices.
///
/// \ingroup Viewers
class VRViewer : public Viewer {
  public:
    /// The constructor is passed an VRContext that is used to interact with
    /// OpenVR.
    VRViewer(VRContext &vr_context);
    virtual ~VRViewer();

    /// Sets the VRCamera to update.
    void SetCamera(const SG::VRCameraPtr &camera) { camera_ = camera; }

    virtual void Render(const SG::Scene &scene, Renderer &renderer) override;
    virtual void EmitEvents(std::vector<Event> &events) override;

  private:
    /// Context managing VR rendering and input.
    VRContext &vr_context_;

    /// Stores the camera that is used to get the current height offset.
    SG::VRCameraPtr camera_;
};
