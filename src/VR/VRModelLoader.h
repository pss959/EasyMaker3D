//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/Hand.h"
#include "Items/Controller.h"
#include "Util/Memory.h"

namespace SG { DECL_SHARED_PTR(Node); }

// VRModelLoader is an internal static class used by VRSystem to load SteamVR
// controller models and convert them to SG Nodes.
///
/// \ingroup VR
class VRModelLoader {
  public:
    /// Loads the SteamVR controller model with the given handle and stores the
    /// resulting data in the Controller::CustomModel instance. Returns true if
    /// successful. This assumes that OpenVR has been initialized successfully.
    /// The Hand is supplied for setting up connection points.
    static bool LoadControllerModel(uint64_t handle, Hand hand,
                                    Controller::CustomModel &custom_model);
};
