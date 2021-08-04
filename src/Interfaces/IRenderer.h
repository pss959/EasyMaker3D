#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#include "Interfaces/IInterfaceBase.h"

class  IScene;
struct View;

//! Abstract Interface class defining an OpenGL-based renderer.
//! \ingroup Interfaces
class IRenderer : public IInterfaceBase {
  public:
    //! Optional framebuffer target information passed to RenderScene().
    struct FBTarget {
        // Framebuffer indices.
        int target_fb = -1;  //! Target framebuffer index.
        int color_fb  = -1;  //! Source framebuffer index for color texture.
        int depth_fb  = -1;  //! Source framebuffer index for depth texture.
    };

    //! Returns the current X11 Display;
    virtual Display     * GetDisplay()  const = 0;

    //! Returns the current GLXContext.
    virtual GLXContext    GetContext()  const = 0;

    //! Returns the current GLXDrawable.
    virtual GLXDrawable   GetDrawable() const = 0;

    //! Creates a framebuffer that can be used as a render target, returning
    //! its index.
    virtual int           CreateFramebuffer() = 0;

    //! Renders the given scene using the given view. If fb_target is not null,
    //! it is used instead of the default target.
    virtual void RenderScene(IScene &scene, const View &view,
                             const FBTarget *fb_target = nullptr) = 0;
};
