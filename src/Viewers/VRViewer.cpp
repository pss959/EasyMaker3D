#include "Viewers/VRViewer.h"

#include "SG/VRCamera.h"
#include "Util/Assert.h"

VRViewer::VRViewer(const RenderFunc &render_func, const EmitFunc &emit_func) {
    ASSERT(render_func);
    ASSERT(emit_func);
    render_func_ = render_func;
    emit_func_   = emit_func;
}

VRViewer::~VRViewer() {
}

void VRViewer::Render(const SG::Scene &scene, IRenderer &renderer) {
    render_func_(scene, renderer);
}

void VRViewer::EmitEvents(std::vector<Event> &events) {
    emit_func_(events);
}
