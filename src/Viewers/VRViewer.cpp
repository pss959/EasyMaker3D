#include "Viewers/VRViewer.h"

#include "Assert.h"
#include "SG/VRCamera.h"
#include "VR/VRContext.h"
#include "VR/VRInput.h"

VRViewer::VRViewer(VRContext &context) : context_(context) {
}

VRViewer::~VRViewer() {
}

void VRViewer::Init() {
    // Initialize input.
    input_.reset(new VRInput(context_));
}

void VRViewer::Render(const SG::Scene &scene, Renderer &renderer) {
    ASSERT(camera_);
    // Pass the position of the camera to use as the base position.
    context_.Render(scene, renderer, camera_->GetPosition());
}

void VRViewer::EmitEvents(std::vector<Event> &events) {
    ASSERT(camera_);
    input_->EmitEvents(events, camera_->GetPosition());
}
