#pragma once

#include "Base/Memory.h"
#include "Items/Controller.h"

namespace SG { DECL_SHARED_PTR(Node); }

// VRModelLoader is an internal static class used by VRContext to load SteamVR
// controller models and convert them to SG Nodes.
///
/// \ingroup App
class VRModelLoader {
  public:
    /// Loads the SteamVR controller model with the given handle and stores the
    /// resulting data in the Controller::CustomModel instance. Returns true if
    /// successful. This assumes that OpenVR has been initialized successfully.
    static bool LoadControllerModel(uint64_t handle,
                                    Controller::CustomModel &custom_model);
};
