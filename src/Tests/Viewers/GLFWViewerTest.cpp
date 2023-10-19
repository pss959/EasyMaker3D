#include <vector>

#include "Base/Event.h"
#include "Base/IWindowSystem.h"
#include "SG/WindowCamera.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Tests/Viewers/FakeRenderer.h"
#include "Viewers/GLFWViewer.h"

// ----------------------------------------------------------------------------
// GLFWViewerTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class GLFWViewerTest : public SceneTestBase {
  protected:
    class TestWindowSystem;
    DECL_SHARED_PTR(TestWindowSystem);

    GLFWViewer          viewer;
    TestWindowSystemPtr tws;

    /// GLFWViewer error messages are stored in this.
    Str                 error_msg;

    /// The constructor sets up a GLFWViewer with an error function that stores
    /// the message and installs a TestWindowSystem in it.
    GLFWViewerTest();

    /// Sets up a WindowCamera in the GLFWViewer.
    void SetCamera();
};

// ----------------------------------------------------------------------------
// GLFWViewerTest::TestWindowSystem class.
// ----------------------------------------------------------------------------

/// Derived IWindowSystem class used for testing without relying on GLFW.
class GLFWViewerTest::TestWindowSystem : public IWindowSystem {
  public:
    /// Set this to the value returned by Init().
    bool init_succeeds = true;

    /// Set this to true to have WasWindowClosed() return true.
    bool simulate_window_close = false;

    /// Set this to true for IsShiftKeyPressed() to return true.
    bool is_shift_pressed = false;

    Vector2i win_size{ 0, 0 };
    Vector2i win_pos{ 0, 0 };
    bool     is_fullscreen = false;
    size_t   pre_render_count = 0;
    size_t   post_render_count = 0;
    size_t   flush_count = 0;
    EventOptions exp_options;

    virtual bool Init(const ErrorFunc &error_func) override {
        return init_succeeds;
    }
    virtual void Terminate() override  {}
    virtual bool CreateWindow(const Vector2i &size, const Str &title) {
        win_size = size;
        return true;
    };
    virtual void SetWindowPosition(const Vector2i &pos) override {
        win_pos = pos;
    }
    virtual void SetFullScreen() override { is_fullscreen = true; }
    virtual Vector2i GetWindowSize() override { return win_size; }
    virtual Vector2i GetFramebufferSize() override { return win_size; }
    virtual void PreRender() override { ++pre_render_count; }
    virtual void PostRender() override { ++post_render_count; }
    virtual bool WasWindowClosed() override { return simulate_window_close; }
    virtual void RetrieveEvents(const EventOptions &options,
                                std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override { ++flush_count; }
    virtual bool IsShiftKeyPressed() override { return is_shift_pressed; }
};

void GLFWViewerTest::TestWindowSystem::RetrieveEvents(
    const EventOptions &options, std::vector<Event> &events) {
    EXPECT_EQ(exp_options.wait_for_events,     options.wait_for_events);
    EXPECT_EQ(exp_options.ignore_mouse_motion, options.ignore_mouse_motion);

    // Do nothing if simulating exit.
    if (simulate_window_close)
        return;

    // Create 4 events that should get compressed to 2.
    Event ev;
    ev.device = Event::Device::kMouse;

    // Event 0: Mouse button press.
    ev.flags.Set(Event::Flag::kButtonPress);
    ev.flags.Set(Event::Flag::kPosition2D);
    ev.button = Event::Button::kMouse1;
    ev.position2D.Set(0, 0);
    events.push_back(ev);

    // Event 1: Mouse motion. Should be compressed.
    ev = Event();
    ev.device = Event::Device::kMouse;
    ev.flags.Set(Event::Flag::kPosition2D);
    ev.position2D.Set(.1f, 0);
    events.push_back(ev);

    // Event 2: Mouse motion. Should be compressed.
    ev.position2D.Set(.2f, 0);
    events.push_back(ev);

    // Event 3: Mouse button release.
    ev.flags.Set(Event::Flag::kButtonRelease);
    ev.button = Event::Button::kMouse1;
    ev.position2D.Set(.2f, 0);
    events.push_back(ev);
}

// ----------------------------------------------------------------------------
// GLFWViewerTest functions.
// ----------------------------------------------------------------------------

GLFWViewerTest::GLFWViewerTest() :
    viewer([&](const Str &msg){ error_msg = msg; }), tws(new TestWindowSystem) {
    viewer.SetWindowSystem(tws);
}

void GLFWViewerTest::SetCamera() {
    auto cam = ParseTypedObject<SG::WindowCamera>(R"(
    WindowCamera {
      position: 1 2 3,
      orientation: 0 1 0 30,
      fov: 45,
      near: 2,
      far: 10,
    })");

    viewer.SetCamera(cam);
}

// ----------------------------------------------------------------------------
// GLFWViewerTest tests.
// ----------------------------------------------------------------------------

TEST_F(GLFWViewerTest, Init) {
    EXPECT_EQ(Vector2i::Zero(), tws->win_size);
    EXPECT_EQ(Vector2i::Zero(), tws->win_pos);
    EXPECT_EQ(Vector2i::Zero(), tws->GetWindowSize());
    EXPECT_EQ(Vector2i::Zero(), tws->GetFramebufferSize());
    EXPECT_FALSE(tws->is_fullscreen);

    EXPECT_TRUE(viewer.Init(Vector2i(800, 600), true));
    EXPECT_EQ(Vector2i(800, 600), tws->win_size);
    EXPECT_EQ(Vector2i(600, 100), tws->win_pos);
    EXPECT_EQ(Vector2i(800, 600), tws->GetWindowSize());
    EXPECT_EQ(Vector2i(800, 600), tws->GetFramebufferSize());
    EXPECT_TRUE(tws->is_fullscreen);
}

TEST_F(GLFWViewerTest, InitFail) {
    tws->init_succeeds = false;
    EXPECT_FALSE(viewer.Init(Vector2i(600, 600), false));
}

TEST_F(GLFWViewerTest, Render) {
    SetCamera();
    FakeRenderer renderer;
    auto scene = CreateObject<SG::Scene>();
    EXPECT_EQ(0U, renderer.render_count);
    EXPECT_EQ(0U, tws->pre_render_count);
    EXPECT_EQ(0U, tws->post_render_count);
    viewer.Render(*scene, renderer);
    EXPECT_EQ(1U, renderer.render_count);
    EXPECT_EQ(1U, tws->pre_render_count);
    EXPECT_EQ(1U, tws->post_render_count);
}

TEST_F(GLFWViewerTest, Camera) {
    viewer.Init(Vector2i(600, 600), false);  // Square aspect ratio.
    EXPECT_EQ(Vector2i(600, 600), viewer.GetWindowSize());

    // Camera->Frustum.
    SetCamera();
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
    std::vector<Event> events;

    // This should create 4 events and compress them to 2.
    viewer.EmitEvents(events);
    EXPECT_EQ(2U, events.size());

    EXPECT_TRUE(events[0].flags.Has(Event::Flag::kButtonPress));
    EXPECT_TRUE(events[0].flags.Has(Event::Flag::kPosition2D));
    EXPECT_ENUM_EQ(Event::Device::kMouse,  events[0].device);
    EXPECT_ENUM_EQ(Event::Button::kMouse1, events[0].button);
    EXPECT_EQ(Point2f(0, 0),               events[0].position2D);

    EXPECT_TRUE(events[1].flags.Has(Event::Flag::kButtonRelease));
    EXPECT_TRUE(events[1].flags.Has(Event::Flag::kPosition2D));
    EXPECT_ENUM_EQ(Event::Device::kMouse,  events[1].device);
    EXPECT_ENUM_EQ(Event::Button::kMouse1, events[1].button);
    EXPECT_EQ(Point2f(.2f, 0),             events[1].position2D);

    EXPECT_EQ(0U, tws->flush_count);
    viewer.FlushPendingEvents();
    EXPECT_EQ(1U, tws->flush_count);
}

TEST_F(GLFWViewerTest, Shift) {
    EXPECT_FALSE(viewer.IsShiftKeyPressed());
    tws->is_shift_pressed = true;
    EXPECT_TRUE(viewer.IsShiftKeyPressed());
}

TEST_F(GLFWViewerTest, Close) {
    std::vector<Event> events;

    // Also test EventOptions setting.
    viewer.SetPollEventsFlag(false);
    viewer.EnableMouseMotionEvents(false);
    tws->exp_options.wait_for_events     = true;
    tws->exp_options.ignore_mouse_motion = true;

    // This should add an exit event.
    tws->simulate_window_close = true;
    viewer.EmitEvents(events);
    EXPECT_EQ(1U, events.size());
    EXPECT_ENUM_EQ(Event::Device::kMouse,  events[0].device);
    EXPECT_TRUE(events[0].flags.Has(Event::Flag::kExit));
}
