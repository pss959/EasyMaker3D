#include "Renderer.h"

#include <iostream>

#include <ion/gfx/image.h>
#include <ion/gfx/sampler.h>
#include <ion/math/transformutils.h>

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
#include "SG/Visitor.h"
#include "Util/OutputMuter.h"
#include "View.h"

// XXXX Check all of these...
using ion::gfx::FramebufferObject;
using ion::gfx::Image;
using ion::gfx::Sampler;
using ion::gfx::StateTable;
using ion::gfx::Uniform;
using ion::math::Anglef;
using ion::math::Matrix4f;
using ion::math::Point2i;
using ion::math::Point3f;
using ion::math::Range1f;
using ion::math::Range2i;
using ion::math::Vector2f;
using ion::math::Vector2i;
using ion::math::Vector3f;
using ion::math::Vector4f;

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
    // XXXX AddNodeTracking(root_);
    // XXXX AddNodeTracking(depth_map_root_);
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

#if XXXX
void Renderer::SetUpShadowPass_(const SG::Scene &scene,
                                const SG::RenderPass &pass,
                                const SG::PointLight &light) {
    // Compute the distance from the light source to the origin.
    const Point3f &light_pos = light.GetPosition();
    //const float    light_dist = ion::math::Length(light_pos - Point3f::Zero());

    // Compute the bias matrix. Use a perspective view from the light source
    // position.
    const Anglef kFov = Anglef::FromDegrees(60.f);

    const Matrix4f proj_mat =
        // XXXX Constants?
        ion::math::PerspectiveMatrixFromView(kFov, 1.f, .01f, 200.f) *
        ion::math::LookAtMatrixFromCenter(light_pos, Point3f::Zero(),
                                          Vector3f::AxisY());
    const Matrix4f bias_mat =
        ion::math::TranslationMatrix(Vector3f(.5f, .5f, .5f)) *
        (ion::math::ScaleMatrixH(Vector3f(.5f, .5f, .5f)) * proj_mat);

    ASSERT(pass.GetRootNode());
    const ion::gfx::NodePtr &root = pass.GetRootNode()->GetIonNode();
    ASSERT(root);

    root->SetUniformByName("uBiasMatrix", bias_mat);
    root->SetUniformByName("uLightPos",   light_pos);
    root->SetUniformByName("uProjectionMatrix", Matrix4f::Identity());
    root->SetUniformByName("uModelviewMatrix", Matrix4f::Identity());

    // Prepare for shadow passes. Disable all Ion nodes for SG nodes with
    // shader programs. This makes sure all nodes use the depth shader.
    SG::Visitor visitor;
    visitor.Visit(pass.GetRootNode(),
                  [](const SG::NodePtr &node){
                  if (node->GetShaderProgram()) {
                      node->GetIonNode()->Enable(false);
                      return SG::Visitor::TraversalCode::kPrune;
                  }
                  return SG::Visitor::TraversalCode::kContinue;});

    renderer_->BindFramebuffer(pass.GetFBO());
    renderer_->DrawScene(root);

    // Re-enable Ion nodes.
    visitor.Visit(pass.GetRootNode(),
                  [](const SG::NodePtr &node){
                  if (node->GetShaderProgram())
                      node->GetIonNode()->Enable(true);
                  return SG::Visitor::TraversalCode::kContinue;});
}
#endif

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
