#pragma once

#include "SG/Scene.h"
#include "Tests/Testing.h"
#include "Viewers/IRenderer.h"

/// Fake implementation of IRenderer interface.
///
/// \ingroup Tests
class FakeRenderer : public IRenderer {
  public:
    /// This is true between BeginFrame() and EndFrame().
    bool in_frame = false;

    /// Counts the calls to Render().
    size_t render_count = 0;

    virtual void Reset(const SG::Scene &scene) override {
        frame_count_ = 0;
        render_count = 0;
    }

    virtual void BeginFrame() override {
        EXPECT_FALSE(in_frame);
        ++frame_count_;
        in_frame = true;
    }
    virtual void EndFrame() override {
        EXPECT_TRUE(in_frame);
        in_frame = false;
    }

    virtual uint64 GetFrameCount() const override { return frame_count_; }

    virtual void SetFBTarget(const FBTargetPtr &fb_target) override {}

    virtual void RenderScene(const SG::Scene &scene,
                             const Frustum &frustum) override {
        ++render_count;
    }

    virtual ion::gfx::ImagePtr ReadImage(const Viewport &rect) override {
        return ion::gfx::ImagePtr();
    }

    virtual uint32 GetResolvedTextureID() override { return 0; }

  private:
    uint32 frame_count_ = 0;
};
