#include "Viewers/VRViewer.h"

#include "SG/VRCamera.h"
#include "Util/Assert.h"
#include "App/VRContext.h"

VRViewer::VRViewer(VRContext &vr_context) : vr_context_(vr_context) {
}

VRViewer::~VRViewer() {
}

void VRViewer::Render(const SG::Scene &scene, Renderer &renderer) {
    ASSERT(camera_);
    // Pass the position of the camera to use as the base position.
    vr_context_.Render(scene, renderer, camera_->GetCurrentPosition());
}

void VRViewer::EmitEvents(std::vector<Event> &events) {
    ASSERT(camera_);
    vr_context_.EmitEvents(events, camera_->GetCurrentPosition());
}
