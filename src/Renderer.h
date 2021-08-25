#pragma once

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/node.h>
#include <ion/gfx/renderer.h>
#include <ion/gfx/shaderprogram.h>
#include <ion/gfx/texture.h>
#include <ion/gfxutils/frame.h>
#include <ion/gfxutils/shadermanager.h>

#if ENABLE_ION_REMOTE
#include <ion/remote/nodegraphhandler.h>
#include <ion/remote/remoteserver.h>
#endif

#include "Interfaces/IRenderer.h"

namespace SG {
class Scene;
class RenderPass;
class PointLight;
}

//! Renderer is an implementation of the IRenderer interface.
class Renderer : public IRenderer {
  public:
    Renderer(const ion::gfxutils::ShaderManagerPtr &shader_manager,
             bool use_ion_remote);
    virtual ~Renderer();

    virtual const char * GetClassName() const override { return "Renderer"; }
    virtual Display     * GetDisplay()  const override;
    virtual GLXContext    GetContext()  const override;
    virtual GLXDrawable   GetDrawable() const override;
    virtual int           CreateFramebuffer() override;
    virtual void RenderScene(const SG::Scene &scene, const View &view,
                             const FBTarget *fb_target = nullptr) override;

  private:
    Display       *display_;   //! Current X11 Display.
    GLXContext     context_;   //! Current GLXContext.
    GLXDrawable    drawable_;  //! Current GLXDrawable.

    ion::gfx::RendererPtr           renderer_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::gfxutils::FramePtr         frame_;
    bool                            is_remote_enabled_ = false;

    void SetUpShadowPass_(const SG::Scene &scene,
                          const SG::RenderPass &pass,
                          const SG::PointLight &light);

#if ENABLE_ION_REMOTE
    //! Stores the remote server used for Ion debugging.
    std::unique_ptr<ion::remote::RemoteServer> remote_;
    //! Stores the NodeGraphHandler used for Ion debugging.
    ion::remote::NodeGraphHandlerPtr ngh_;

    //! Sets up the remote server used for Ion debugging.
    void SetUpRemoteServer_();

    //! Adds the given node to the set tracked by Ion debugging, if it is not
    //! already tracked.
    void AddNodeTracking(const ion::gfx::NodePtr &node);
#endif
};
