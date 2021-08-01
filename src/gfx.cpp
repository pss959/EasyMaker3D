#include "gfx.h"

// Work around GL incompatibilities.
#undef GL_GLEXT_VERSION

#include <iostream>

#include "ion/gfx/node.h"
#include "ion/gfx/renderer.h"

#if ENABLE_ION_REMOTE
#include <ion/remote/nodegraphhandler.h>
#include <ion/remote/remoteserver.h>
#include <ion/remote/remoteserver.h>
#include <ion/remote/resourcehandler.h>
#include <ion/remote/settinghandler.h>
#include <ion/remote/shaderhandler.h>
#include "ion/remote/tracinghandler.h"
#endif

#include "scene.h"

// ----------------------------------------------------------------------------
// Ion/OpenGL tracing.
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
// Helper_ class definition.
// ----------------------------------------------------------------------------

class GFX::Helper_ {
  public:
    Helper_();
    Display     * GetDisplay()  const { return display_;  }
    GLXContext    GetContext()  const { return context_;  }
    GLXDrawable   GetDrawable() const { return drawable_; }
    int           CreateFramebuffer();
    void          Draw(int width, int height);
    void          DrawWithInfo(const RenderInfo &info);

  private:
    // Cache these in case someone else changes them.
    Display       *display_;
    GLXContext     context_;
    GLXDrawable    drawable_;

    ion::gfx::RendererPtr           renderer_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::gfxutils::FramePtr         frame_;

    Scene                           scene_;

#if ENABLE_ION_REMOTE
    std::unique_ptr<ion::remote::RemoteServer> remote_;
#endif
    void SetUpRemoteServer_();
};

// ----------------------------------------------------------------------------
// GFX class.
// ----------------------------------------------------------------------------

GFX::GFX() : helper_(new Helper_()) {
}

GFX::~GFX() {
}

Display     * GFX::GetDisplay()  const { return helper_->GetDisplay();  }
GLXContext    GFX::GetContext()  const { return helper_->GetContext();  }
GLXDrawable   GFX::GetDrawable() const { return helper_->GetDrawable(); }

int GFX::CreateFramebuffer() {
    return helper_->CreateFramebuffer();
}

void GFX::Draw(int width, int height) const {
    helper_->Draw(width, height);
}

void GFX::DrawWithInfo(const RenderInfo &info) const {
    helper_->DrawWithInfo(info);
}

// ----------------------------------------------------------------------------
// Helper_ class functions.
// ----------------------------------------------------------------------------

#if GLDEBUG_ENABLED
// XXXX
static void GLMessageCallback(GLenum source, GLenum type,
                              GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message,
                              const void* userParam) {
    std::cerr << "GL " << (type == GL_DEBUG_TYPE_ERROR ? "ERROR " : "MSG")
              << " type = " << type
              << " severity = " << severity
              << ": " << message << "\n";
}
#endif

GFX::Helper_::Helper_() {
    display_  = XOpenDisplay(nullptr);
    context_  = glXGetCurrentContext();
    drawable_ = glXGetCurrentDrawable();

#if GLDEBUG_ENABLED
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLMessageCallback, 0);  // XXXX
#endif

    ion::gfx::GraphicsManagerPtr manager(new ion::gfx::GraphicsManager);
    manager->EnableErrorChecking(true);
    renderer_.Reset(new ion::gfx::Renderer(manager));
    shader_manager_.Reset(new ion::gfxutils::ShaderManager);
    frame_.Reset(new ion::gfxutils::Frame);
    SetUpRemoteServer_();
}

int GFX::Helper_::CreateFramebuffer() {
    GLuint fb;
    renderer_->GetGraphicsManager()->GenFramebuffers(1, &fb);
    return fb;
}

void GFX::Helper_::Draw(int width, int height) {
    glXMakeCurrent(GetDisplay(), GetDrawable(), GetContext());

    scene_.SetViewport(
        ion::math::Range2i::BuildWithSize(ion::math::Point2i(0, 0),
                                          ion::math::Vector2i(width, height)));

    TRACE_START_
    ion::gfx::GraphicsManager &gm = *renderer_->GetGraphicsManager();
    gm.BindFramebuffer(GL_FRAMEBUFFER, 0);
    renderer_->DrawScene(scene_.GetRoot());
    TRACE_END_
}

void GFX::Helper_::DrawWithInfo(const RenderInfo &info) {
    glXMakeCurrent(GetDisplay(), GetDrawable(), GetContext());

    TRACE_START_

    ion::gfx::GraphicsManager &gm = *renderer_->GetGraphicsManager();
    gm.BindFramebuffer(GL_FRAMEBUFFER, info.fb);

    gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, info.color_fb, 0);
    gm.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_TEXTURE_2D, info.depth_fb, 0);

    scene_.SetViewport(info.viewport_rect);
    scene_.SetProjection(info.projection);
    scene_.SetView(info.view);
    renderer_->DrawScene(scene_.GetRoot());

    TRACE_END_
}

void GFX::Helper_::SetUpRemoteServer_() {
#if ENABLE_ION_REMOTE
#if XXXX_REMOTE_MESSES_UP_STEAMVR
    remote_.reset(new ion::remote::RemoteServer(1234));

    ion::remote::NodeGraphHandlerPtr ngh(new ion::remote::NodeGraphHandler);
    ngh->AddNode(scene_root_);
    remote_->RegisterHandler(ngh);
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
#endif
}
