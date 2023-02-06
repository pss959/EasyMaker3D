#pragma once

#include <functional>

#include "Base/IEmitter.h"
#include "Base/Memory.h"
#include "Math/Types.h"
#include "Viewers/Viewer.h"

DECL_SHARED_PTR(VRViewer);
namespace SG { DECL_SHARED_PTR(VRCamera); }

/// VRViewer is a derived Viewer and IEmitter to view in VR and produce events
/// from VR devices. It is passed the functions (attached to the VRContext)
/// that perform the rendering and event emitting.
///
/// \ingroup Viewers
class VRViewer : public Viewer, public IEmitter {
  public:
    /// Type for a function used to render into the VRViewer. It is passed the
    /// Scene, Renderer, and a base position for VR viewing.
    typedef std::function<void(const SG::Scene &, Renderer &,
                               const Point3f &)> RenderFunc;

    /// Type for a function used to emit events. It is passed the base position
    /// for VR viewing.
    typedef std::function<void(std::vector<Event> &, const Point3f &)> EmitFunc;

    /// The constructor is passed a function used to render and a function used
    /// to emit events.
    /// the VR system.
    VRViewer(const RenderFunc &render_func, const EmitFunc &emit_func);
    virtual ~VRViewer();

    /// Sets the VRCamera to update.
    void SetCamera(const SG::VRCameraPtr &camera) { camera_ = camera; }

    virtual void Render(const SG::Scene &scene, Renderer &renderer) override;

    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override {}

  private:
    /// Function used to render into the VR headset.
    RenderFunc      render_func_;

    /// Function used to emit VR-related events.
    EmitFunc        emit_func_;

    /// Stores the camera that is used to get the current height offset.
    SG::VRCameraPtr camera_;
};
