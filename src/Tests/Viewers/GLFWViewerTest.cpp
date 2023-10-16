#include "Base/IWindowSystem.h"
#include "SG/WindowCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Viewers/GLFWViewer.h"

/// \ingroup Tests
class GLFWViewerTest : public SceneTestBase {
  public:
    /// Derived IWindowSystem class used for testing without relying on GLFW.
    class TestWindowSystem : public IWindowSystem {
        virtual bool Init(const ErrorFunc &error_func) override {
            return true;
        }
        virtual void Terminate() override  {}
        virtual bool CreateWindow(const Vector2i &size, const Str &title) {
            win_size_ = size;
            return true;
        };
        virtual void SetWindowPosition(const Vector2i &pos) override {}
        virtual void SetFullScreen() override {}
        virtual Vector2i GetWindowSize() override { return win_size_; }
        virtual Vector2i GetFramebufferSize() override { return win_size_; }
        virtual void PreRender() override {}
        virtual void PostRender() override {}
        virtual bool WasWindowClosed() override { return false; }
        virtual void RetrieveEvents(const EventOptions &options,
                                    std::vector<Event> &events) override {
        }
        virtual void FlushPendingEvents() override {}
        virtual bool IsShiftKeyPressed() override { return false; }
      private:
        Vector2i win_size_;
    };
    DECL_SHARED_PTR(TestWindowSystem);
};

TEST_F(GLFWViewerTest, Camera) {
    auto cam = ParseTypedObject<SG::WindowCamera>(R"(
    WindowCamera {
      position: 1 2 3,
      orientation: 0 1 0 30,
      fov: 45,
      near: 2,
      far: 10,
    })");

    TestWindowSystemPtr tws(new TestWindowSystem);

    GLFWViewer viewer;
    viewer.SetWindowSystem(tws);
    viewer.Init(Vector2i(600, 600), false);  // Square aspect ratio.
    EXPECT_EQ(Vector2i(600, 600), viewer.GetWindowSize());

    // Camera->Frustum.
    viewer.SetCamera(cam);
    EXPECT_NOT_NULL(viewer.GetFrustum());
    const auto &fr = *viewer.GetFrustum();
    const Anglef a = Anglef::FromDegrees(22.5f);
    const auto rot = BuildRotation(0, 1, 0, 30);
    EXPECT_EQ(Vector2i(600, 600), fr.viewport.GetSize());
    EXPECT_EQ(Point3f(1, 2, 3),   fr.position);
    EXPECT_EQ(rot,                fr.orientation);
    EXPECT_EQ(-a,                 fr.fov_left);
    EXPECT_EQ( a,                 fr.fov_right);
    EXPECT_EQ(-a,                 fr.fov_down);
    EXPECT_EQ( a,                 fr.fov_up);
    EXPECT_EQ(2,                  fr.pnear);
    EXPECT_EQ(10,                 fr.pfar);
}

TEST_F(GLFWViewerTest, Events) {
    // Call trivial stuff.
    GLFWViewer viewer;
    viewer.SetPollEventsFlag(true);
    viewer.EnableMouseMotionEvents(true);
    EXPECT_FALSE(viewer.IsShiftKeyPressed());
}
