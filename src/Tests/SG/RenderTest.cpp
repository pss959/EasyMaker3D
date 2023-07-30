#if 1 // XXXX
#include <ion/gfx/tests/fakeglcontext.h>
#include <ion/gfx/tests/fakegraphicsmanager.h>
#include <ion/gfx/renderer.h>
#include <ion/portgfx/glcontext.h>
#endif

#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/RenderData.h"
#include "SG/Scene.h"
#include "Tests/SceneTestBase.h"

class RenderTest : public SceneTestBase {
  protected:
    void UpdateNodeForRenderPass(const SG::RenderPass &pass, SG::Node &node) {
        // Let the node update its enabled flags and anything else it needs.
        node.UpdateForRenderPass(pass.GetName());

        // Nothing to do if the node is disabled for traversal.
        if (node.IsEnabled()) {
            // Recurse even if kRender is disabled; it does not apply to
            // children.
            for (const auto &child: node.GetAllChildren())
                UpdateNodeForRenderPass(pass, *child);
        }
    }
};

TEST_F(RenderTest, RealScene) {
    // This is required for font setup.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    const std::string input = ReadDataFile("RealScene");
    auto scene = ReadScene(input, true);

    const size_t kWidth  = 1280;
    const size_t kHeight = 1024;

    auto gl_context = ion::gfx::testing::FakeGlContext::Create(kWidth, kHeight);
    ion::portgfx::GlContext::MakeCurrent(gl_context);
    ion::gfx::testing::FakeGraphicsManagerPtr graphics_mgr(
        new ion::gfx::testing::FakeGraphicsManager);
    ion::gfx::RendererPtr renderer(new ion::gfx::Renderer(graphics_mgr));
    // Set up a RenderData.
    SG::RenderData data;
    Frustum        frustum;
    data.viewport    = frustum.viewport;
    data.proj_matrix = GetProjectionMatrix(frustum);
    data.view_matrix = GetViewMatrix(frustum);
    data.view_pos    = frustum.position;
    const auto &lights = scene->GetLights();
    data.per_light.resize(lights.size());
    for (size_t i = 0; i < lights.size(); ++i) {
        auto &pl         = data.per_light[i];
        pl.position      = lights[i]->GetPosition();
        pl.color         = lights[i]->GetColor();
        pl.casts_shadows = lights[i]->CastsShadows();
        pl.light_matrix  = Matrix4f::Identity();
    }
    data.root_node = scene->GetRootNode();
    ASSERT(data.root_node);

    // Process each RenderPass.
    const FBTarget *fb_target = nullptr;
    for (const auto &pass: scene->GetRenderPasses()) {
        // Update all pass-dependent nodes under the RenderPass's root.
        UpdateNodeForRenderPass(*pass, *scene->GetRootNode());

        // Render the pass.
        pass->Render(*renderer, data, fb_target);
    }
}
