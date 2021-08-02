#pragma once

#include "ion/gfx/node.h"
#include "ion/gfx/renderer.h"
#include "ion/gfxutils/frame.h"
#include "ion/gfxutils/shadermanager.h"

#if ENABLE_ION_REMOTE
#include <ion/remote/nodegraphhandler.h>
#include <ion/remote/remoteserver.h>
#endif

#include "Interfaces/IRenderer.h"

//! Renderer is an implementation of the IRenderer interface.
class Renderer : public IRenderer {
  public:
    Renderer();
    ~Renderer();

    virtual Display     * GetDisplay()  const override { return display_;  }
    virtual GLXContext    GetContext()  const override { return context_;  }
    virtual GLXDrawable   GetDrawable() const override { return drawable_; }
    virtual int           CreateFramebuffer() override;
    virtual void RenderScene(IScene &scene,
                             const ion::math::Range2i &viewport) override;
    virtual void RenderSceneToTarget(IScene &scene,
                                     const Target &target) override;

  private:
    Display       *display_;   //! Current X11 Display.
    GLXContext     context_;   //! Current GLXContext.
    GLXDrawable    drawable_;  //! Current GLXDrawable.

    ion::gfx::RendererPtr           renderer_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::gfxutils::FramePtr         frame_;

#if ENABLE_ION_REMOTE
    //! Stores the remote server used for Ion debugging.
    std::unique_ptr<ion::remote::RemoteServer> remote_;
    //! Stores the NodeGraphHandler used for Ion debugging.
    ion::remote::NodeGraphHandlerPtr ngh_;

    //! Sets up the remote server used for Ion debugging.
    void SetUpRemoteServer_();
#endif

    //! Adds the given node to the set tracked by Ion debugging, if it is not
    //! already tracked.
    void AddNodeTracking(const ion::gfx::NodePtr &node);
};
