#pragma once

#include <vector>

#include "Memory.h"

class Renderer;
struct Event;

DECL_SHARED_PTR(Viewer);

namespace SG { class Scene; }

/// Abstract base class defining a viewer that can both render a scene to a
/// device and emit events related to that device.
///
/// \ingroup Viewers
class Viewer {
  public:
    virtual ~Viewer() {}

    /// Renders a Scene using the given renderer.
    virtual void Render(const SG::Scene &scene, Renderer &renderer) = 0;

    /// Adds emitted Event instances to the given vector.
    virtual void EmitEvents(std::vector<Event> &events) = 0;

    /// Flushes events that may be pending. This is used to prevent events that
    /// were generated before a reload or reset from being processed after the
    /// objects they affected have been deleted. The base class defines this to
    /// do nothing.
    virtual void FlushPendingEvents() {}
};
