#include "Base/Event.h"
#include "Handlers/MainHandler.h"
#include "Items/Controller.h"
#include "Math/Types.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Trackers/TestGrippable.h"
#include "Tests/Trackers/TestTouchable.h"
#include "Trackers/MouseTracker.h"
#include "Util/Delay.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/IScrollable.h"

/// \ingroup Tests
class MainHandlerTest : public SceneTestBase {
  protected:
    /// Derived GenericWidget with IScrollable interface for testing valuator
    /// events.
    class TestScrollableWidget : public GenericWidget, public IScrollable {
      public:
        float last_delta = 0;

        // IScrollable interface.
        virtual bool ProcessValuator(float delta) override {
            last_delta = delta;
            return true;
        }
      protected:
        TestScrollableWidget() {}
        friend class Parser::Registry;
    };
    DECL_SHARED_PTR(TestScrollableWidget);

    /// Shorter MouseTracker click timeout to speed up tests.
    static constexpr float kTimeout = .001f;

    /// Sets the MouseTracker click timeout to #kTimeout while an instance of
    /// this is in scope and restores it to its usual value afterwards.
    class TimeoutShortener {
      public:
        TimeoutShortener()  { MouseTracker::SetClickTimeout(kTimeout); }
        ~TimeoutShortener() { MouseTracker::SetClickTimeout(0); }
    };

    PrecisionStorePtr       prec;
    TestScrollableWidgetPtr widget;

    /// Sets up a MainHandler with a Context and PrecisionStore and accesses
    /// the GenericWidget.
    void InitHandler(MainHandler &mh);

    /// Returns a mouse press or release event. The default \p x and \p y will
    /// hit the GenericWidget.
    Event GetWidgetEvent(bool is_press, float x = .5f, float y = .5f);

    /// Returns a mouse drag event to the given point.
    Event GetDragEvent(float x, float y);
};

void MainHandlerTest::InitHandler(MainHandler &mh) {
    static const Str kContents = R"(
  children: [
    Node {
      TEMPLATES: [
        <"nodes/templates/RadialMenu.emd">,
        <"nodes/templates/Controller.emd">,
      ],
      children: [
        <"nodes/Controllers.emd">,
        CLONE "T_RadialMenu" "LeftMenu"  {},
        CLONE "T_RadialMenu" "RightMenu" {},

        # TestScrollableWidget to test clicking/dragging/valuator.
        TestScrollableWidget "TestWidget" { shapes: [ Box {} ] },

        # Button to test non-draggable widget.
        PushButtonWidget "TestButton" {
          translation: 0 4 0,
          shapes: [ Box {} ]
        },
      ],
    }
  ]
)";
    Parser::Registry::AddType<TestScrollableWidget>("TestScrollableWidget");

    FrustumPtr frustum(new Frustum);
    auto scene = ReadRealScene(kContents);
    auto lc = SG::FindTypedNodeInScene<Controller>(*scene, "LeftController");
    auto rc = SG::FindTypedNodeInScene<Controller>(*scene, "RightController");
    widget  = SG::FindTypedNodeInScene<TestScrollableWidget>(*scene,
                                                             "TestWidget");

    // Set up and install a MainHandler::Context.
    MainHandler::Context mc;
    mc.scene            = scene;
    mc.frustum          = frustum;
    mc.path_to_stage    = SG::NodePath(scene->GetRootNode());
    mc.left_controller  = lc;
    mc.right_controller = rc;
    mh.SetContext(mc);

    // Create and install a PrecisionStore.
    prec.reset(new PrecisionStore);
    mh.SetPrecisionStore(prec);
}

Event MainHandlerTest::GetWidgetEvent(bool is_press, float x, float y) {
    Event event;
    event.device = Event::Device::kMouse;
    event.flags.Set(is_press ? Event::Flag::kButtonPress :
                    Event::Flag::kButtonRelease);
    event.flags.Set(Event::Flag::kPosition2D);
    event.button = Event::Button::kMouse1;
    event.position2D.Set(x, y);
    return event;
}

Event MainHandlerTest::GetDragEvent(float x, float y) {
    Event event;
    event.device = Event::Device::kMouse;
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(x, y);
    return event;
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(MainHandlerTest, Defaults) {
    MainHandler mh(false);  // No VR.

    EXPECT_ENUM_EQ(Event::Device::kUnknown, mh.GetActiveDevice());
    EXPECT_ENUM_EQ(Event::Device::kUnknown, mh.GetLastActiveDevice());
    EXPECT_TRUE(mh.IsWaiting());
}

TEST_F(MainHandlerTest, Valuator) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);

    // Valuator event with no 2D position.
    Event event;
    event.device = Event::Device::kMouse;
    event.flags.Set(Event::Flag::kPosition1D);
    event.position1D = .2f;
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, mh.HandleEvent(event));

    // Valuator event not over any IScrollable.
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D = Point2f(0, 0);
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, mh.HandleEvent(event));

    // Test a valuator Event over the TestScrollableWidget.
    EXPECT_EQ(0, widget->last_delta);
    event.position2D = Point2f(.5f, .5f);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, mh.HandleEvent(event));
    EXPECT_EQ(.2f, widget->last_delta);
}

TEST_F(MainHandlerTest, Hover) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);

    // Move over the Widget to hover it. MainHandler should not mark this event
    // as handled.
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(.5f, .5f)));
    EXPECT_TRUE(mh.IsWaiting());
    EXPECT_TRUE(widget->IsHovering());

    // Move off of the Widget.
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(0, 0)));
    EXPECT_TRUE(mh.IsWaiting());
    EXPECT_FALSE(widget->IsHovering());
}

TEST_F(MainHandlerTest, Click) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);

    // Set a shorter timeout for the MouseTracker while this is in scope.
    TimeoutShortener ts;

    size_t  click_count = 0;
    Widget *click_widget = nullptr;
    auto click_func = [&](const ClickInfo &info){
        ++click_count;
        click_widget = info.widget;
    };
    mh.GetClicked().AddObserver("key", click_func);

    EXPECT_EQ(0U, click_count);
    EXPECT_NULL(click_widget);

    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_EQ(0U, click_count);  // Not a click until release.
    EXPECT_FALSE(mh.IsWaiting());

    // Wait long enough for this to be processed as a click and update the
    // MainHandler so it times out.
    Util::DelayThread(kTimeout + .0001f);
    mh.ProcessUpdate(false);

    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(false)));
    EXPECT_EQ(1U, click_count);
    EXPECT_EQ(widget.get(), click_widget);
    EXPECT_TRUE(mh.IsWaiting());

    // Wait after the click deactivation this time and make sure the click is
    // still processed.
    click_count  = 0;
    click_widget = nullptr;
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_EQ(0U, click_count);
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(false)));
    EXPECT_EQ(0U, click_count);  // Not counted until timeout.
    EXPECT_NULL(click_widget);
    EXPECT_FALSE(mh.IsWaiting());
    Util::DelayThread(kTimeout + .0001f);
    mh.ProcessUpdate(false);
    EXPECT_EQ(1U, click_count);
    EXPECT_EQ(widget.get(), click_widget);
    EXPECT_TRUE(mh.IsWaiting());

    // Test using a PathFilter to not hit the Widget so there is no click.
    mh.SetPathFilter([](const SG::NodePath &){ return false; });
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetWidgetEvent(true)));
    Util::DelayThread(kTimeout + .0001f);
    mh.ProcessUpdate(false);
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetWidgetEvent(false)));
    EXPECT_EQ(1U, click_count);
    EXPECT_EQ(widget.get(), click_widget);
    EXPECT_TRUE(mh.IsWaiting());
}

TEST_F(MainHandlerTest, DoubleClick) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);

    // Set a shorter timeout for the MouseTracker while this is in scope.
    TimeoutShortener ts;

    size_t  click_count  = 0;
    Widget *click_widget = nullptr;
    bool    is_mod_click = false;
    auto click_func = [&](const ClickInfo &info){
        ++click_count;
        click_widget = info.widget;
        is_mod_click = info.is_modified_mode;
    };
    mh.GetClicked().AddObserver("key", click_func);

    EXPECT_EQ(0U, click_count);
    EXPECT_NULL(click_widget);
    EXPECT_FALSE(is_mod_click);

    // First click.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_EQ(0U, click_count);  // Not a click until release.
    EXPECT_NULL(click_widget);
    EXPECT_FALSE(is_mod_click);
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(false)));

    // Second click.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_EQ(0U, click_count);  // Not a click until release.
    EXPECT_NULL(click_widget);
    EXPECT_FALSE(is_mod_click);
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(false)));

    // Wait so the click finishes.
    Util::DelayThread(kTimeout + .0001f);
    mh.ProcessUpdate(false);

    // Double-click should turn on modified-click mode.
    EXPECT_EQ(1U, click_count);
    EXPECT_EQ(widget.get(), click_widget);
    EXPECT_TRUE(is_mod_click);
    EXPECT_TRUE(mh.IsWaiting());
}

TEST_F(MainHandlerTest, Drag) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);

    // Detect drags applied to the GenericWidget.
    size_t drag_start_count = 0;
    size_t drag_count       = 0;
    size_t drag_end_count   = 0;
    widget->GetDragged().AddObserver(
        "key", [&](const DragInfo *info, bool is_start){
            if (! info)
                ++drag_end_count;
            else if (is_start)
                ++drag_start_count;
            else
                ++drag_count;
        });

    EXPECT_EQ(0U, drag_start_count);
    EXPECT_EQ(0U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // Mouse press to start drag; not detected as a drag until motion.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_EQ(0U, drag_start_count);
    EXPECT_EQ(0U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // Not enough motion to cause a drag.
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(.501f, .5f)));
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_EQ(0U, drag_start_count);
    EXPECT_EQ(0U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // Enough motion to cause a drag.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetDragEvent(.6f, .5f)));
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_EQ(1U, drag_start_count);
    EXPECT_EQ(1U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // More motion.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetDragEvent(.8f, .5f)));
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_EQ(1U, drag_start_count);
    EXPECT_EQ(2U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // Mouse release to end drag.
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(false)));
    EXPECT_EQ(1U, drag_start_count);
    EXPECT_EQ(2U, drag_count);
    EXPECT_EQ(1U, drag_end_count);

    // No drag can happen on a non-DraggableWidget (PushButtonWidget).
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(true, .5f, .8f)));
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(.6f, .8f)));
    EXPECT_EQ(1U, drag_start_count);
    EXPECT_EQ(2U, drag_count);
    EXPECT_EQ(1U, drag_end_count);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(false, .6f, .8f)));
}

TEST_F(MainHandlerTest, Reset) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop,
                   mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_FALSE(mh.IsWaiting());
    mh.Reset();
    EXPECT_TRUE(mh.IsWaiting());
}

TEST_F(MainHandlerTest, VR) {
    MainHandler mh(true);  // Enable VR.
    InitHandler(mh);

    Parser::Registry::AddType<TestGrippable>("TestGrippable");
    auto tg = CreateObject<TestGrippable>();
    TestTouchablePtr tt(new TestTouchable);
    mh.AddGrippable(tg);
    mh.SetTouchable(tt);

    // Repeat the hover test.
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(.5f, .5f)));
    EXPECT_TRUE(mh.IsWaiting());
    EXPECT_TRUE(widget->IsHovering());
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(0, 0)));
    EXPECT_TRUE(mh.IsWaiting());
    EXPECT_FALSE(widget->IsHovering());

    // Repeat with a path filter.
    mh.SetPathFilter([](const SG::NodePath &){ return false; });
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(.5f, .5f)));
    EXPECT_TRUE(mh.IsWaiting());
    EXPECT_FALSE(widget->IsHovering());
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled,
                   mh.HandleEvent(GetDragEvent(0, 0)));
    EXPECT_TRUE(mh.IsWaiting());
    EXPECT_FALSE(widget->IsHovering());

    // Update to make sure the Grippable is processed.
    mh.ProcessUpdate(false);
}
