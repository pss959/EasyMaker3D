#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Enums/Hand.h"
#include "Items/Controller.h"
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

    /// This tries to load a SteamVR controller model for the given Hand. If
    /// successful, this stores the data for the model in the given
    /// Controller::CustomModel and returns true. This should not be called
    /// before InitSystem() or if InitSystem() returns false.
    bool LoadControllerModel(Hand hand, Controller::CustomModel &model);

    /// Sets the Controllers to use for VR. This should not be called before
    /// InitSystem().
    void SetControllers(const ControllerPtr &l_controller,
                        const ControllerPtr &r_controller);

    /// Initializes rendering. This must be called once before the first call
    /// to Render().
    void InitRendering(Renderer &renderer);

    /// Renders.
    void Render(const SG::Scene &scene, Renderer &renderer,
                const Point3f &base_position);

    /// Emits events from all input devices.
    void EmitEvents(std::vector<Event> &events, const Point3f &base_position);

    /// Returns true if the headset is currently being worn.
    bool IsHeadSetOn() const;

    /// Shuts VR down gracefully.
    void Shutdown();

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};
