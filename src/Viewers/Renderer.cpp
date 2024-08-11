//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Viewers/Renderer.h"

#if ENABLE_DEBUG_FEATURES
#  define DO_REMOTE_ 1
#else
#  define DO_REMOTE_ 0
#endif

#include <iostream>

#include <ion/gfx/node.h>
#include <ion/gfx/renderer.h>
#include <ion/gfxutils/frame.h>
#include <ion/gfxutils/shadermanager.h>

#if DO_REMOTE_
#include <ion/remote/nodegraphhandler.h>
#include <ion/remote/remoteserver.h>
#include <ion/remote/resourcehandler.h>
#include <ion/remote/settinghandler.h>
#include <ion/remote/shaderhandler.h>
#include <ion/remote/tracinghandler.h>
#endif

#include "Base/FBTarget.h"
#include "Math/Frustum.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "SG/PointLight.h"
#include "SG/RenderData.h"
#include "SG/RenderPass.h"
#include "SG/Scene.h"
#include "Util/Assert.h"

#include <ion/gfx/graphicsmanager.h>

// ----------------------------------------------------------------------------
// Renderer::Impl_ class.
// ----------------------------------------------------------------------------

/// This class does most of the work for the Renderer.
class Renderer::Impl_ {
  public:
    Impl_(const ion::gfxutils::ShaderManagerPtr &shader_manager,
          bool use_ion_remote);

    void Reset(const SG::Scene &scene);
    void BeginFrame() { frame_->Begin(); }
    void EndFrame()   { frame_->End();   }
    uint64 GetFrameCount() const { return frame_->GetCounter(); }
    void SetFBTarget(const FBTargetPtr &fb_target) { fb_target_ = fb_target; }
    void RenderScene(const SG::Scene &scene, const Frustum &frustum);
    ion::gfx::ImagePtr ReadImage(const Viewport &rect);
    uint32 GetResolvedTextureID();

  private:
    FBTargetPtr                     fb_target_;
    ion::gfx::RendererPtr           renderer_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::gfxutils::FramePtr         frame_;
    bool                            is_remote_enabled_ = false;

    /// Recursive function that updates a Node for rendering the given pass.
    /// This enables or disables the Ion Node based on the Node's flags and
    /// enables or disables Ion UniformBlocks based on their pass selector.
    void UpdateNodeForRenderPass_(const SG::RenderPass &pass, SG::Node &node);

    void SetUpShadowPass_(const SG::Scene &scene,
                          const SG::RenderPass &pass,
                          const SG::PointLight &light);

#if DO_REMOTE_
    /// Stores the remote server used for Ion debugging.
    std::unique_ptr<ion::remote::RemoteServer> remote_;
    /// Stores the NodeGraphHandler used for Ion debugging.
    ion::remote::NodeGraphHandlerPtr ngh_;

    /// Sets up the remote server used for Ion debugging.
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

    ion::gfx::GraphicsManagerPtr manager(new ion::gfx::GraphicsManager);
    manager->EnableErrorChecking(true);
    renderer_.Reset(new ion::gfx::Renderer(manager));
    frame_.Reset(new ion::gfxutils::Frame);

#if DO_REMOTE_
    SetUpRemoteServer_();
#endif
}

void Renderer::Impl_::Reset(const SG::Scene &scene) {
    frame_->ResetCounter();

#if DO_REMOTE_
    if (is_remote_enabled_) {
        ngh_->ClearNodes();
        ASSERT(scene.GetRootNode()->GetIonNode());
        ngh_->AddNode(scene.GetRootNode()->GetIonNode());
        ASSERT(ngh_->GetTrackedNodeCount() == 1U);
    }
#endif
}

void Renderer::Impl_::RenderScene(const SG::Scene &scene,
                                  const Frustum &frustum) {
    // Set up a RenderData.
    SG::RenderData data;
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
    data.root_node = scene.GetRootNode();
    ASSERT(data.root_node);

    // Process each RenderPass.
    for (const auto &pass: scene.GetRenderPasses()) {
        // Update all pass-dependent nodes under the RenderPass's root.
        UpdateNodeForRenderPass_(*pass, *scene.GetRootNode());
        // Render the pass.
        renderer_->PushDebugMarker(pass->GetDesc());
        pass->Render(*renderer_, data, fb_target_.get());
        renderer_->PopDebugMarker();
    }
}

ion::gfx::ImagePtr Renderer::Impl_::ReadImage(const Viewport &rect) {
    ASSERT(renderer_);
    renderer_->BindFramebuffer(fb_target_ ? fb_target_->GetResolvedFBO() :
                               ion::gfx::FramebufferObjectPtr());

    return renderer_->ReadImage(rect, ion::gfx::Image::Format::kRgb888,
                                ion::base::AllocatorPtr());
}

uint32 Renderer::Impl_::GetResolvedTextureID() {
    ASSERT(renderer_);
    if (fb_target_ && fb_target_->IsInitialized()) {
        auto &ca = fb_target_->GetResolvedFBO()->GetColorAttachment(0);
        ASSERT(ca.GetTexture().Get());
        return renderer_->GetResourceGlId(ca.GetTexture().Get());
    }
    return 0;
}

void Renderer::Impl_::UpdateNodeForRenderPass_(const SG::RenderPass &pass,
                                               SG::Node &node) {
    // Let the node update its enabled flags and anything else it needs.
    node.UpdateForRenderPass(pass.GetName());

    // Nothing to do if the node is disabled for traversal.
    if (node.IsEnabled()) {
        // Recurse even if kRender is disabled; it does not apply to children.
        for (const auto &child: node.GetAllChildren())
            UpdateNodeForRenderPass_(pass, *child);
    }
}

#if DO_REMOTE_
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
#endif  // DO_REMOTE_

// ----------------------------------------------------------------------------
// Renderer class implementation.
// ----------------------------------------------------------------------------

Renderer::Renderer(const ion::gfxutils::ShaderManagerPtr &shader_manager,
                   bool use_ion_remote) : impl_(new Impl_(shader_manager,
                                                          use_ion_remote)) {
}

Renderer::~Renderer() {
}

void Renderer::Reset(const SG::Scene &scene) {
    impl_->Reset(scene);
}

void Renderer::BeginFrame() {
    impl_->BeginFrame();
}

void Renderer::EndFrame() {
    impl_->EndFrame();
}

uint64 Renderer::GetFrameCount() const {
    return impl_->GetFrameCount();
}

void Renderer::SetFBTarget(const FBTargetPtr &fb_target) {
    impl_->SetFBTarget(fb_target);
}

void Renderer::RenderScene(const SG::Scene &scene, const Frustum &frustum) {
    impl_->RenderScene(scene, frustum);
}

ion::gfx::ImagePtr Renderer::ReadImage(const Viewport &rect) {
    return impl_->ReadImage(rect);
}

uint32 Renderer::GetResolvedTextureID() {
    return impl_->GetResolvedTextureID();
}
