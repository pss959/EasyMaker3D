//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Util/Memory.h"

struct Event;

namespace SG { class Scene; }

/// Interface defining an object that can emit events.
///
/// \ingroup Base
class IEmitter {
  public:
    /// Adds emitted Event instances to the given vector.
    virtual void EmitEvents(std::vector<Event> &events) = 0;

    /// Flushes events that may be pending. This is used to prevent events that
    /// were generated before a reload or reset from being processed after the
    /// objects they affected have been deleted.
    virtual void FlushPendingEvents() = 0;
};

DECL_SHARED_PTR(IEmitter);
