#include "Renderer.h"

#include <iostream>

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
#include <ion/remote/resourcehandler.h>
#include <ion/remote/settinghandler.h>
#include <ion/remote/shaderhandler.h>
#include <ion/remote/tracinghandler.h>
#endif

#include "Assert.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "SG/PointLight.h"
#include "SG/RenderPass.h"
#include "SG/Scene.h"
#include "SG/ShaderNode.h"

// ----------------------------------------------------------------------------
// Renderer::Impl_ class.
// ----------------------------------------------------------------------------

//! This class does most of the work for the Renderer.
class Renderer::Impl_ {
  public:
    Impl_(const ion::gfxutils::ShaderManagerPtr &shader_manager,
          bool use_ion_remote);

    Display *   GetDisplay()  const { return display_;  }
    GLXContext  GetContext()  const { return context_;  }
    GLXDrawable GetDrawable() const { return drawable_; }

    int CreateFramebuffer();
    void Reset(const SG::Scene &scene);
    uint64_t GetFrameCount() const { return frame_->GetCounter(); }
    void RenderScene(const SG::Scene &scene, const Frustum &frustum,
                     const FBTarget *fb_target = nullptr);

  private:
    Display       *display_;   //!< Current X11 Display.
    GLXContext     context_;   //!< Current GLXContext.
    GLXDrawable    drawable_;  //!< Current GLXDrawable.

    ion::gfx::RendererPtr           renderer_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::gfxutils::FramePtr         frame_;
    bool                            is_remote_enabled_ = false;

    //! Recursive function that updates a Node for rendering the given pass.
    //! This enables or disables the Ion Node based on the Node's flags and
    //! enables or disables Ion UniformBlocks based on their pass selector.
    void UpdateNodeForRenderPass_(const SG::RenderPass &pass, SG::Node &node);

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
#endif
};

// ----------------------------------------------------------------------------
// Renderer::Impl_ implementation.
// ----------------------------------------------------------------------------

Renderer::Impl_::Impl_(const ion::gfxutils::ShaderManagerPtr &shader_manager,
                       bool use_ion_remote) :
    shader_manager_(shader_manager), is_remote_enabled_(use_ion_remote) {
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

int Renderer::Impl_::CreateFramebuffer() {
    GLuint fb;
    renderer_->GetGraphicsManager()->GenFramebuffers(1, &fb);
    return fb;
}

void Renderer::Impl_::Reset(const SG::Scene &scene) {
    frame_->ResetCounter();

#if ENABLE_ION_REMOTE
    if (is_remote_enabled_) {
        ngh_->ClearNodes();
        for (const auto &pass: scene.GetRenderPasses()) {
            ASSERT(pass->GetRootNode()->GetIonNode());
            ngh_->AddNode(pass->GetRootNode()->GetIonNode());
        }
        ASSERT(ngh_->GetTrackedNodeCount() == scene.GetRenderPasses().size());
    }
#endif
}

void Renderer::Impl_::RenderScene(const SG::Scene &scene, const Frustum &frustum,
                                  const FBTarget *fb_target) {
    glXMakeCurrent(GetDisplay(), GetDrawable(), GetContext());

    frame_->Begin();

    // Set up a PassData.
    SG::RenderPass::PassData data;
    data.viewport    = frustum.viewport;
    data.proj_matrix = GetProjectionMatrix(frustum);
    data.view_matrix = GetViewMatrix(frustum);
    data.view_pos    = frustum.position;
    const auto &lights = scene.GetLights();
    data.per_light.resize(lights.size());
    for (size_t i = 0; i < lights.size(); ++i) {
        auto &pl         = data.per_light[i];
        pl.position      = lights[i]->GetPosition();
        pl.color         = lights[i]->GetColor();
        pl.casts_shadows = lights[i]->CastsShadows();
        pl.light_matrix  = Matrix4f::Identity();
    }
    data.fb_target = fb_target;

    // Process each RenderPass.
    for (const auto &pass: scene.GetRenderPasses()) {
        // Let the RenderPass set values in its UniformBlock.
        pass->SetUniforms(data);
        // Update all pass-dependent nodes under the RenderPass's root.
        UpdateNodeForRenderPass_(*pass, *pass->GetRootNode());
        // Render the pass.
        renderer_->PushDebugMarker(pass->GetDesc());
        pass->Render(*renderer_, data);
        renderer_->PopDebugMarker();
    }

    frame_->End();
}

void Renderer::Impl_::UpdateNodeForRenderPass_(const SG::RenderPass &pass,
                                               SG::Node &node) {
    // Each of these updates if necessary.
    node.GetModelMatrix();
    node.GetBounds();

    // Enable or disable the Ion node for rendering.
    ASSERT(node.GetIonNode());
    const bool is_enabled = node.IsEnabled(SG::Node::Flag::kTraversal) &&
        node.IsEnabled(SG::Node::Flag::kRender);
    auto &ion_node = node.GetIonNode();
    ion_node->Enable(is_enabled);

    // Nothing else to do if the node is disabled.
    if (! is_enabled)
        return;

    // Install the Ion ShaderProgram if this is a ShaderNode and the pass
    // matches. Otherwise, temporarily install a null ShaderProgram. This is
    // the best we can do since Ion ShaderPrograms cannot be disabled.
    if (node.GetTypeName() == "ShaderNode") {
        SG::ShaderNode &shader_node = static_cast<SG::ShaderNode &>(node);
        ion_node->SetShaderProgram(
            shader_node.GetPassName() == pass.GetName() ?
            shader_node.GetShaderProgram()->GetIonShaderProgram() :
            ion::gfx::ShaderProgramPtr());
    }

    // Enable or disable all pass-specific UniformBlocks.
    for (const auto &block: node.GetUniformBlocks()) {
        auto &ion_block = block->GetIonUniformBlock();
        ASSERT(ion_block);
        ion_block->Enable(block->GetName().empty() ||
                          block->GetName() == pass.GetName());
    }

    // Recurse.
    for (const auto &child: node.GetChildren())
        UpdateNodeForRenderPass_(pass, *child);
}

#if ENABLE_ION_REMOTE
void Renderer::Impl_::SetUpRemoteServer_() {
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
#endif // ENABLE_ION_REMOTE

// ----------------------------------------------------------------------------
// Renderer class implementation.
// ----------------------------------------------------------------------------

Renderer::Renderer(const ion::gfxutils::ShaderManagerPtr &shader_manager,
                   bool use_ion_remote) : impl_(new Impl_(shader_manager,
                                                          use_ion_remote)) {
}

Renderer::~Renderer() {
}

Display * Renderer::GetDisplay() const {
    return impl_->GetDisplay();
}

GLXContext Renderer::GetContext() const {
    return impl_->GetContext();
}

GLXDrawable Renderer::GetDrawable() const {
    return impl_->GetDrawable();
}

int Renderer::CreateFramebuffer() {
    return impl_->CreateFramebuffer();
}

void Renderer::Reset(const SG::Scene &scene) {
    impl_->Reset(scene);
}

uint64_t Renderer::GetFrameCount() const {
    return impl_->GetFrameCount();
}

void Renderer::RenderScene(const SG::Scene &scene, const Frustum &frustum,
                           const FBTarget *fb_target) {
    impl_->RenderScene(scene, frustum, fb_target);
}
