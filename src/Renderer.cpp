#include "Renderer.h"

#include <iostream>

#if ENABLE_ION_REMOTE
#include <ion/remote/resourcehandler.h>
#include <ion/remote/settinghandler.h>
#include <ion/remote/shaderhandler.h>
#include <ion/remote/tracinghandler.h>
#endif

#include "Assert.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/RenderPass.h"
#include "SG/Scene.h"
#include "View.h"

using ion::math::Matrix4f;
using ion::math::Vector2f;

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
    std::cerr << "GL trace: ========================================\n"       \
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

Renderer::Renderer(const ion::gfxutils::ShaderManagerPtr &shader_manager,
                   bool use_ion_remote) :
    shader_manager_(shader_manager),
    is_remote_enabled_(use_ion_remote) {
    ASSERT(shader_manager);

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

void Renderer::RenderScene(const SG::Scene &scene, const View &view,
                           const FBTarget *fb_target) {
#if ENABLE_ION_REMOTE
    for (const auto &pass: scene.GetRenderPasses())
        AddNodeTracking(pass->GetRootNode()->GetIonNode());
#endif

    // Make sure the scene is updated.
    scene.Update();

    glXMakeCurrent(GetDisplay(), GetDrawable(), GetContext());

    frame_->Begin();
    TRACE_START_;

    SG::RenderPass::PassData data;
    data.viewport    = view.GetViewport();
    data.proj_matrix = view.GetProjectionMatrix();
    data.view_matrix = view.GetViewMatrix();
    const auto &lights = scene.GetLights();
    data.per_light.resize(lights.size());
    for (size_t i = 0; i < lights.size(); ++i) {
        auto &pl       = data.per_light[i];
        pl.position    = lights[i]->GetPosition();
        pl.color       = lights[i]->GetColor();
        pl.bias_matrix = Matrix4f::Identity();
        pl.depth_range = Vector2f(0.f, 1.f);
    }
    // Process each RenderPass in the scene.
    for (const auto &pass: scene.GetRenderPasses()) {
        pass->Render(*renderer_, data);
    }

    TRACE_END_;
    frame_->End();
}

#if ENABLE_ION_REMOTE
void Renderer::SetUpRemoteServer_() {
    if (! is_remote_enabled_)
        return;
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
}

void Renderer::AddNodeTracking(const ion::gfx::NodePtr &node) {
    ASSERT(node.Get());
    if (is_remote_enabled_ && ! ngh_->IsNodeTracked(node))
        ngh_->AddNode(node);
}
#endif // ENABLE_ION_REMOTE
