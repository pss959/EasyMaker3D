#pragma once

#include <memory>

#include "SG/Typedefs.h"
#include "Viewers/Viewer.h"

class VRContext;
class VRInput;

/// VRViewer is a derived Viewer that uses OpenXR to view in VR and produce
/// events from VR devices.
class VRViewer : public Viewer {
  public:
    /// The constructor is passed an VRContext that is used to interact with
    /// OpenVR.
    VRViewer(VRContext &context);
    virtual ~VRViewer();

    /// Initializes the viewer.
    void Init();

    /// Sets the VRCamera to update.
    void SetCamera(const SG::VRCameraPtr &camera) { camera_ = camera; }

    virtual void Render(const SG::Scene &scene, Renderer &renderer) override;
    virtual void EmitEvents(std::vector<Event> &events) override;

  private:
    /// Context used for OpenVR.
    VRContext &context_;

    /// Stores the camera that is used to get the current height offset.
    SG::VRCameraPtr camera_;

    /// Handles all VR input to produce events.
    std::unique_ptr<VRInput> input_;
};

typedef std::shared_ptr<VRViewer> VRViewerPtr;
