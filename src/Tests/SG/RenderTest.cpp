#include <ion/gfx/tests/fakeglcontext.h>
#include <ion/gfx/tests/fakegraphicsmanager.h>
#include <ion/gfx/renderer.h>
#include <ion/portgfx/glcontext.h>

#include "Base/FBTarget.h"
#include "Math/Frustum.h"
#include "Math/Linear.h"
#include "SG/Box.h"
#include "SG/Node.h"
#include "SG/RenderData.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class RenderTest : public SceneTestBase {
  protected:
    const size_t kWidth  = 1280;  ///< Framebuffer width.
    const size_t kHeight = 1024;  ///< Framebuffer height.

    /// Sets up to render the scene read from the named data file.
    SG::ScenePtr SetUpScene(const Str &file_name);

    /// Renders the scene using the given FBTarget, which may be null.
    void RenderScene(const FBTarget *fb_target);

  private:
    SG::ScenePtr          scene_;
    SG::RenderData        data_;
    ion::gfx::RendererPtr renderer_;

    void UpdateNodeForRenderPass_(const SG::RenderPass &pass, SG::Node &node);
};

SG::ScenePtr RenderTest::SetUpScene(const Str &file_name) {
    const Str input = ReadDataFile(file_name);
    scene_ = ReadScene(input, true);

    auto gl_context = ion::gfx::testing::FakeGlContext::Create(kWidth, kHeight);
    ion::portgfx::GlContext::MakeCurrent(gl_context);
    ion::gfx::testing::FakeGraphicsManagerPtr graphics_mgr(
        new ion::gfx::testing::FakeGraphicsManager);
    graphics_mgr->EnableErrorChecking(true);
    renderer_.Reset(new ion::gfx::Renderer(graphics_mgr));

    // Set up the RenderData.
    Frustum frustum;
    data_.viewport    = frustum.viewport;
    data_.proj_matrix = GetProjectionMatrix(frustum);
    data_.view_matrix = GetViewMatrix(frustum);
    data_.view_pos    = frustum.position;
    const auto &lights = scene_->GetLights();
    data_.per_light.resize(lights.size());
    for (size_t i = 0; i < lights.size(); ++i) {
        auto &pl         = data_.per_light[i];
        pl.position      = lights[i]->GetPosition();
        pl.color         = lights[i]->GetColor();
        pl.casts_shadows = lights[i]->CastsShadows();
        pl.light_matrix  = Matrix4f::Identity();
    }
    data_.root_node = scene_->GetRootNode();
    ASSERT(data_.root_node);

    return scene_;
}

void RenderTest::RenderScene(const FBTarget *fb_target) {
    ASSERT(scene_);

    // Process each RenderPass.
    for (const auto &pass: scene_->GetRenderPasses()) {
        // Update all pass-dependent nodes under the RenderPass's root.
        UpdateNodeForRenderPass_(*pass, *scene_->GetRootNode());

        // Render the pass.
        pass->Render(*renderer_, data_, fb_target);
    }
}

void RenderTest::UpdateNodeForRenderPass_(const SG::RenderPass &pass,
                                          SG::Node &node) {
    // Let the node update its enabled flags and anything else it needs.
    node.UpdateForRenderPass(pass.GetName());

    // Nothing to do if the node is disabled for traversal.
    if (node.IsEnabled()) {
        // Recurse even if kRender is disabled; it does not apply to
        // children.
        for (const auto &child: node.GetAllChildren())
            UpdateNodeForRenderPass_(pass, *child);
    }
}

// ----------------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------------

TEST_F(RenderTest, RealScene) {
    auto scene = SetUpScene("RealScene.emd");

    // Add a TextNode to test text rendering.
    auto text = CreateObject<SG::TextNode>();
    text->SetText("Original");
    scene->GetRootNode()->AddChild(text);

    // Add a node to test RenderPass management.
    auto pass_node = ParseTypedObject<SG::Node>(
        R"(Node { shader_names: ["ShadowDepth"] })");
    scene->GetRootNode()->AddChild(pass_node);

    // Add a render-disabled node with a Shape to test shape disabling.
    auto disabled_node = CreateObject<SG::Node>();
    disabled_node->AddShape(CreateObject<SG::Box>());
    disabled_node->SetFlagEnabled(SG::Node::Flag::kRender, false);
    scene->GetRootNode()->AddChild(disabled_node);

    RenderScene(nullptr);

    // Enable the disabled Node and re-render to test shape management. Also
    // make sure that the TextNode is rebuilt when necessary.
    disabled_node->SetFlagEnabled(SG::Node::Flag::kRender, true);
    text->SetText("Changed");

    RenderScene(nullptr);
}

TEST_F(RenderTest, RealSceneWithFBTarget) {
    FBTarget fb_target;
    fb_target.Init("RenderTest", Vector2ui(kWidth, kHeight), 4);

    SetUpScene("RealScene.emd");
    RenderScene(&fb_target);
}
