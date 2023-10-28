#include "Base/Event.h"
#include "SG/Scene.h"
#include "SG/VRCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Viewers/FakeRenderer.h"
#include "Viewers/IRenderer.h"
#include "Viewers/VRViewer.h"

/// \ingroup Tests
class VRViewerTest : public SceneTestBase {};

TEST_F(VRViewerTest, RenderAndEmit) {
    // VRViewer is a thin wrapper around two callback functions. Make sure they
    // are called properly.

    FakeRenderer renderer;
    auto         scene = CreateObject<SG::Scene>();
    auto         cam   = ParseTypedObject<SG::VRCamera>(
        "VRCamera { base_position: 1 2 3 }");

    size_t emit_count = 0;

    auto render_func = [&](const SG::Scene &s, IRenderer &r){
        EXPECT_EQ(scene.get(), &s);
        EXPECT_EQ(&renderer,   &r);
        Frustum f;
        renderer.RenderScene(s, f);
    };
    auto emit_func = [&](std::vector<Event> &e){
        EXPECT_TRUE(e.empty());
        ++emit_count;
    };

    VRViewer viewer(render_func, emit_func);
    viewer.SetCamera(cam);

    // Test rendering.
    EXPECT_EQ(0U, renderer.render_count);
    viewer.Render(*scene, renderer);
    EXPECT_EQ(1U, renderer.render_count);

    // Test emitting events.
    EXPECT_EQ(0U, emit_count);
    std::vector<Event> events;
    viewer.EmitEvents(events);
    EXPECT_EQ(1U, emit_count);

    // Should have no effect.
    viewer.FlushPendingEvents();
}
