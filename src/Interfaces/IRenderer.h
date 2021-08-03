#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/vector.h>

#include "Interfaces/IInterfaceBase.h"

class IScene;

//! Abstract Interface class defining an OpenGL-based renderer.
//! \ingroup Interfaces
class IRenderer : public IInterfaceBase {
  public:
    //! Information passed to RenderToTarget().
    struct Target {
        // Viewport to render to.
        ion::math::Range2i viewport;

        // Projection and view matrices.
        ion::math::Matrix4f projection_matrix;
        ion::math::Matrix4f view_matrix;

        // Framebuffer indices.
        int target_fb = -1;  //! Target framebuffer index.
        int color_fb  = -1;  //! Source framebuffer index for color texture.
        int depth_fb  = -1;  //! Source framebuffer index for depth texture.

        Target() {
            projection_matrix = ion::math::Matrix4f::Identity();
            view_matrix       = ion::math::Matrix4f::Identity();
        }
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

    //! Renders a Scene to the current GLXDrawable using the standard
    //! framebuffer and the given viewport rectangle.
    virtual void RenderScene(IScene &scene,
                             const ion::math::Range2i &viewport) = 0;

    //! Renders a Scene to the current GLXDrawable using the information in the
    //! given Target instance.
    virtual void RenderSceneToTarget(IScene &scene, const Target &target) = 0;
};
