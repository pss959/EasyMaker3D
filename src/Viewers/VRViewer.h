#pragma once

#include <functional>

#include "Base/IEmitter.h"
#include "Viewers/IVRSystem.h"
#include "Viewers/Viewer.h"

DECL_SHARED_PTR(VRViewer);

/// VRViewer is a derived Viewer and IEmitter to view in VR and produce events
/// from VR devices. It is passed the functions (attached to the IVRSystem)
/// that perform the rendering and event emitting.
///
/// \ingroup Viewers
class VRViewer : public Viewer, public IEmitter {
  public:
    /// The constructor is passed an IVRSystem instance that does most of the
    /// work.
    explicit VRViewer(const IVRSystemPtr &vr_system);

    virtual void Render(const SG::Scene &scene, IRenderer &renderer) override;
    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override {}

  private:
    IVRSystemPtr vr_system_;
};
