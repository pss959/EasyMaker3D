#pragma once

#include "Base/Memory.h"

class IRenderer;
DECL_SHARED_PTR(Viewer);
namespace SG { class Scene; }

/// Abstract base class defining a viewer that can render a scene to a device.
///
/// \ingroup Viewers
class Viewer {
  public:
    virtual ~Viewer() {}

    /// Renders a Scene using the given renderer.
    virtual void Render(const SG::Scene &scene, IRenderer &renderer) = 0;
};
