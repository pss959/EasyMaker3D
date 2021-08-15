#include "Renderer.h"

#include <assert.h>

#include <iostream>

#if ENABLE_ION_REMOTE
#include <ion/remote/resourcehandler.h>
#include <ion/remote/settinghandler.h>
#include <ion/remote/shaderhandler.h>
#include <ion/remote/tracinghandler.h>
#endif

#include "View.h"

// ----------------------------------------------------------------------------
// Optional Ion/OpenGL tracing.
// ----------------------------------------------------------------------------

// Turn this on to trace GL calls from Ion.
#define TRACE_GL_ 0

#if TRACE_GL_
#  define TRACE_START_                                                  \
    ion::gfx::TracingStream &s =                                        \
        renderer_->GetGraphicsManager()->GetTracingStream();            \
    s.StartTracing();
#  define TRACE_END_                                                          \
    s.StopTracing();                                                          \
    std::cerr << "XXXX GL trace: ===================================\n"       \
              << s.String()                                                   \
              << "==================================================\n";      \
    s.Clear();
#else
#  define TRACE_START_
#  define TRACE_END_
#endif

// ----------------------------------------------------------------------------
// Renderer class implementation.
// ----------------------------------------------------------------------------

Renderer::Renderer(const ion::gfxutils::ShaderManagerPtr shader_manager) :
    shader_manager_(shader_manager) {
    display_  = XOpenDisplay(nullptr);
    context_  = glXGetCurrentContext();
    drawable_ = glXGetCurrentDrawable();

    ion::gfx::GraphicsManagerPtr manager(new ion::gfx::GraphicsManager);
    manager->EnableErrorChecking(true);
    renderer_.Reset(new ion::gfx::Renderer(manager));
    frame_.Reset(new ion::gfxutils::Frame);

#if ENABLE_ION_REMOTE
    SetUpRemoteServer_();
#endif
}

Renderer::~Renderer() {
}

Display *   Renderer::GetDisplay()  const { return display_;  }
GLXContext  Renderer::GetContext()  const { return context_;  }
GLXDrawable Renderer::GetDrawable() const { return drawable_; }

int Renderer::CreateFramebuffer() {
    GLuint fb;
    renderer_->GetGraphicsManager()->GenFramebuffers(1, &fb);
    return fb;
}

void Renderer::RenderView(const View &view, const FBTarget *fb_target) {
    glXMakeCurrent(GetDisplay(), GetDrawable(), GetContext());

    TRACE_START_;

    // Set up the framebuffer(s).
    ion::gfx::GraphicsManager &gm = *renderer_->GetGraphicsManager();
    if (fb_target) {
        assert(fb_target->target_fb >= 0);
        assert(fb_target->color_fb  >  0);
        assert(fb_target->depth_fb  >  0);
        gm.BindFramebuffer(GL_FRAMEBUFFER, fb_target->target_fb);

        gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, fb_target->color_fb, 0);
        gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D, fb_target->depth_fb, 0);
    }
    else {
        gm.BindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    renderer_->DrawScene(view.GetRoot());

    TRACE_END_;

#if ENABLE_ION_REMOTE
    AddNodeTracking(view.GetRoot());
#endif
}

#if ENABLE_ION_REMOTE
void Renderer::SetUpRemoteServer_() {
#if 0  // XXXX REMOTE MESSES UP STEAMVR
    remote_.reset(new ion::remote::RemoteServer(1234));

    ngh_.Reset(new ion::remote::NodeGraphHandler);
    remote_->RegisterHandler(ngh_);
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::ResourceHandler(renderer_)));
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::SettingHandler()));
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::ShaderHandler(shader_manager_, renderer_)));
    remote_->RegisterHandler(
        ion::remote::HttpServer::RequestHandlerPtr(
            new ion::remote::TracingHandler(frame_, renderer_)));
#endif
}

void Renderer::AddNodeTracking(const ion::gfx::NodePtr &node) {
#if 0  // XXXX REMOTE MESSES UP STEAMVR
    if (! ngh_->IsNodeTracked(node))
        ngh_->AddNode(node);
#endif
}
#endif // ENABLE_ION_REMOTE
