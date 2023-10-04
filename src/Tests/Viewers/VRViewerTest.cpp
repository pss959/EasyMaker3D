#include "Base/Event.h"
#include "SG/Scene.h"
#include "SG/VRCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Viewers/IRenderer.h"
#include "Viewers/VRViewer.h"

// XXXX
class FakeRenderer : public IRenderer {
    virtual void Reset(const SG::Scene &scene) override {}
    virtual void BeginFrame() override {}
    virtual void EndFrame() override {}
    virtual uint64 GetFrameCount() const override { return 0; }
    virtual void RenderScene(const SG::Scene &scene, const Frustum &frustum,
                             const FBTarget *fb_target = nullptr) override {}
    virtual uint32 GetResolvedTextureID(const FBTarget &fb_target) override {
        return 0;
    }
    virtual ion::gfx::ImagePtr ReadImage(const Range2i &rect) override {
        return ion::gfx::ImagePtr();
    }
};

// ----------------------------------------------------------------------------

class VRViewerTest : public SceneTestBase {};

TEST_F(VRViewerTest, RenderAndEmit) {
    // VRViewer is a thin wrapper around two callback functions. Make sure they
    // are called properly.

    FakeRenderer renderer;
    auto         scene = CreateObject<SG::Scene>();
    auto         cam   = ParseTypedObject<SG::VRCamera>(
        "VRCamera { base_position: 1 2 3 }");

    size_t render_count = 0;
    size_t emit_count   = 0;

    auto render_func = [&](const SG::Scene &s, IRenderer &r, const Point3f &p){
        EXPECT_EQ(scene.get(),      &s);
        EXPECT_EQ(&renderer,        &r);
        EXPECT_EQ(Point3f(1, 2, 3),  p);
        EXPECT_EQ(0U,                render_count);
        ++render_count;
    };
    auto emit_func = [&](std::vector<Event> &e, const Point3f &p){
        EXPECT_TRUE(e.empty());
        EXPECT_EQ(Point3f(1, 2, 3), p);
        EXPECT_EQ(0U,               emit_count);
        ++emit_count;
    };

    VRViewer viewer(render_func, emit_func);
    viewer.SetCamera(cam);

    // Test rendering.
    viewer.Render(*scene, renderer);

    // Test emitting events.
    std::vector<Event> events;
    viewer.EmitEvents(events);

    // Should have no effect.
    viewer.FlushPendingEvents();
}
