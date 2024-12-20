//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Viewers/VRViewer.h"

#include "Util/Assert.h"

VRViewer::VRViewer(const IVRSystemPtr &vr_system) : vr_system_(vr_system) {
    ASSERT(vr_system);
}

void VRViewer::Render(const SG::Scene &scene, IRenderer &renderer) {
    vr_system_->Render(scene, renderer);
}

void VRViewer::EmitEvents(std::vector<Event> &events) {
    vr_system_->EmitEvents(events);
}
