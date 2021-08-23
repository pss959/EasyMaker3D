#pragma once

#include <ion/math/vector.h>

#include "Interfaces/IInterfaceBase.h"

class IRenderer;
class View;
namespace SG { class Scene; }

//! Abstract Interface class defining a viewer that can use a renderer to
//! render a scene to a viewing device.
//!
//! \ingroup Interfaces
class IViewer : public IInterfaceBase {
  public:
    //! Initializes the viewer using the given window size (if it needs
    //! it). Returns false if anything fails.
    virtual bool Init(const ion::math::Vector2i &new_size) = 0;

    //! Changes the viewing window size. This may be ignored by some viewers.
    virtual void SetSize(const ion::math::Vector2i &new_size) = 0;

    //! Returns the current View. This should work for all viewers.
    virtual View & GetView() = 0;

    //! Renders a Scene using the given renderer.
    virtual void Render(const SG::Scene &scene, IRenderer &renderer) = 0;
};
