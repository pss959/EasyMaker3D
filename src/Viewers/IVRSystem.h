#pragma once

#include <vector>

#include "Enums/Hand.h"
#include "Items/Controller.h"
#include "Math/Types.h"
#include "Util/Memory.h"

struct Event;
class  IRenderer;

namespace SG { class Scene; DECL_SHARED_PTR(VRCamera); }

/// IVRSystem is an interface class that abstracts VR system functions.  It
/// allows for testing without having to use a real VR device and library.
///
/// \ingroup Viewers
class IVRSystem {
  public:
    /// \name Startup and Shutdown
    ///@{

    /// Starts the VR system. Returns false if there is no VR device available
    /// or if anything else goes wrong. This should be called before any other
    /// calls, and no other calls are guaranteed to work if this returns false.
    virtual bool Startup() = 0;

    /// Shuts VR down gracefully.
    virtual void Shutdown() = 0;

    ///@}

    /// \name Controller Handling
    ///@{

    /// This tries to load a 3D controller model for the given Hand, storing
    /// the resulting data in the given Controller::CustomModel. Returns false
    /// on error.
    virtual bool LoadControllerModel(Hand hand,
                                     Controller::CustomModel &model) = 0;

    /// Sets the Controllers to use for VR.
    virtual void SetControllers(const ControllerPtr &l_controller,
                                const ControllerPtr &r_controller) = 0;

    ///@}

    /// Initializes rendering. This must be called once before the first call
    /// to Render().
    virtual void InitRendering(IRenderer &renderer) = 0;

    /// Sets the VR camera used for subsequent rendering and pose computations.
    virtual void SetCamera(const SG::VRCameraPtr &cam) = 0;

    /// Renders.
    virtual void Render(const SG::Scene &scene, IRenderer &renderer) = 0;

    /// Adds events from all VR devices.
    virtual void EmitEvents(std::vector<Event> &events) = 0;

    /// Returns true if the headset is currently being worn.
    virtual bool IsHeadSetOn() const = 0;
};

DECL_SHARED_PTR(IVRSystem);
