#pragma once

#include "Interfaces/IInterfaceBase.h"

class IRenderer;

namespace SG { class Scene; }

//! Abstract Interface class defining a viewer that can use a renderer to
//! render a scene to a viewing device.
//!
//! \ingroup Interfaces
class IViewer : public IInterfaceBase {
  public:
    //! Renders a Scene using the given renderer.
    virtual void Render(const SG::Scene &scene, IRenderer &renderer) = 0;
};
