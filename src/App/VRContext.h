#pragma once

#include <vector>

#include "Math/Types.h"

struct Event;
class  Renderer;
namespace SG { class Scene; }

/// VRContext defines a environment that sets up OpenVR for all VR-related
/// parts of the application. It handles both rendering and input.
///
/// \ingroup App
class VRContext {
  public:
    VRContext();
    ~VRContext();

    /// Initializes the VR system. Returns false if there is no VR device
    /// available or if anything else goes wrong.
    bool InitSystem();

    /// Initializes rendering. This must be called once before the first call
    /// to Render().
    void InitRendering(Renderer &renderer);

    /// Renders.
    void Render(const SG::Scene &scene, Renderer &renderer,
                const Point3f &base_position);

    /// Emits events from all input devices.
    void EmitEvents(std::vector<Event> &events, const Point3f &base_position);

    /// Shuts VR down gracefully.
    void Shutdown();

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};
