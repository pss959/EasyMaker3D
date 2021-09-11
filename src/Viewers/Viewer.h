#pragma once

#include <vector>

class IRenderer;
struct Event;

namespace SG { class Scene; }

//! Abstract base class defining a viewer that can both render a scene to a
//! device and emit events related to that device.
//! \ingroup Viewers
class Viewer {
  public:
    virtual ~Viewer() {}

    //! Renders a Scene using the given renderer.
    virtual void Render(const SG::Scene &scene, IRenderer &renderer) = 0;

    //! Adds emitted Event instances to the given vector.
    virtual void EmitEvents(std::vector<Event> &events) = 0;
};
