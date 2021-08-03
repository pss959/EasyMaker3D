#pragma once

#include <ion/math/vector.h>

class IRenderer;
class IScene;

//! Abstract Interface class defining a viewer that can use a renderer to
//! render a scene to a viewing device.
//! \ingroup Interfaces
class IViewer {
  public:
    //! Initializes the viewer using the given window size (if it needs
    //! it). Returns false if anything fails.
    virtual bool Init(const ion::math::Vector2i &new_size) = 0;

    //! Changes the viewing window size. This may be ignored by some viewers.
    virtual void SetSize(const ion::math::Vector2i &new_size) = 0;

    //! Returns the current viewing size. This should work for all viewers.
    virtual ion::math::Vector2i GetSize() const = 0;

    //! Renders the given scene using the given renderer.
    virtual void Render(IScene &scene, IRenderer &renderer) = 0;
};
