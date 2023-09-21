#include "Base/Event.h"
#include "Handlers/MainHandler.h"
#include "Items/Controller.h"
#include "Math/Types.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Delay.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"

/// \ingroup Tests
class MainHandlerTest : public SceneTestBase {
  protected:
    PrecisionStorePtr prec;
    GenericWidgetPtr  widget;

    /// Sets up a MainHandler with a Context and PrecisionStore and accesses
    /// the GenericWidget.
    void InitHandler(MainHandler &mh);

    /// Returns a mouse press or release event that will hit the GenericWidget.
    Event GetWidgetEvent(bool is_press);

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

        # Button to test clicking.
        GenericWidget "TestWidget" { shapes: [ Box {} ] },
      ],
    }
  ]
)";
    FrustumPtr frustum(new Frustum);
    auto scene = ReadRealScene(kContents);
    auto lc = SG::FindTypedNodeInScene<Controller>(*scene, "LeftController");
    auto rc = SG::FindTypedNodeInScene<Controller>(*scene, "RightController");
    widget  = SG::FindTypedNodeInScene<GenericWidget>(*scene, "TestWidget");

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

Event MainHandlerTest::GetWidgetEvent(bool is_press) {
    Event event;
    event.device = Event::Device::kMouse;
    event.flags.Set(is_press ? Event::Flag::kButtonPress :
                    Event::Flag::kButtonRelease);
    event.flags.Set(Event::Flag::kPosition2D);
    event.button = Event::Button::kMouse1;
    event.position2D.Set(.5f, .5f);
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

    // Detect Valuator changes.
    Event::Device last_dev = Event::Device::kUnknown;
    float         last_val = 0;
    auto val_func = [&](Event::Device dev, float val){
        last_dev = dev;
        last_val = val;
    };
    mh.GetValuatorChanged().AddObserver("key", val_func);

    Event event;
    event.device = Event::Device::kMouse;
    event.flags.Set(Event::Flag::kPosition1D);
    event.position1D = .2f;
    EXPECT_ENUM_EQ(Event::Device::kUnknown,        last_dev);
    EXPECT_EQ(0,                                   last_val);
    EXPECT_TRUE(mh.HandleEvent(event));
    EXPECT_ENUM_EQ(Event::Device::kMouse,          last_dev);
    EXPECT_EQ(.2f,                                 last_val);

    event.device = Event::Device::kLeftController;
    event.position1D = -.1f;
    EXPECT_TRUE(mh.HandleEvent(event));
    EXPECT_ENUM_EQ(Event::Device::kLeftController, last_dev);
    EXPECT_EQ(-.1f,                                last_val);
}

TEST_F(MainHandlerTest, Click) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);

    size_t  click_count = 0;
    Widget *click_widget = nullptr;
    auto click_func = [&](const ClickInfo &info){
        ++click_count;
        click_widget = info.widget;
    };
    mh.GetClicked().AddObserver("key", click_func);

    EXPECT_EQ(0U, click_count);
    EXPECT_NULL(click_widget);

    EXPECT_TRUE(mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_EQ(0U, click_count);  // Not a click until release.
    EXPECT_FALSE(mh.IsWaiting());

    // Wait long enough for this to be processed as a click and update the
    // MainHandler so it times out.
    Util::DelayThread(TK::kMouseClickTimeout + .0001f);
    mh.ProcessUpdate(false);

    EXPECT_TRUE(mh.HandleEvent(GetWidgetEvent(false)));
    EXPECT_EQ(1U, click_count);
    EXPECT_EQ(widget.get(), click_widget);
    EXPECT_TRUE(mh.IsWaiting());

    // Test using a PathFilter to not hit the Widget so there is no click.
    mh.SetPathFilter([](const SG::NodePath &){ return false; });
    EXPECT_TRUE(mh.HandleEvent(GetWidgetEvent(true)));
    Util::DelayThread(TK::kMouseClickTimeout + .0001f);
    mh.ProcessUpdate(false);
    EXPECT_TRUE(mh.HandleEvent(GetWidgetEvent(false)));
    EXPECT_EQ(2U, click_count);
    EXPECT_NULL(click_widget);
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
    EXPECT_TRUE(mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_EQ(0U, drag_start_count);
    EXPECT_EQ(0U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // Enough motion to cause a drag.
    EXPECT_TRUE(mh.HandleEvent(GetDragEvent(.6f, 0)));
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_EQ(1U, drag_start_count);
    EXPECT_EQ(1U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // More motion.
    EXPECT_TRUE(mh.HandleEvent(GetDragEvent(.8f, 0)));
    EXPECT_FALSE(mh.IsWaiting());
    EXPECT_EQ(1U, drag_start_count);
    EXPECT_EQ(2U, drag_count);
    EXPECT_EQ(0U, drag_end_count);

    // Mouse release to end drag.
    EXPECT_TRUE(mh.HandleEvent(GetWidgetEvent(false)));
    EXPECT_EQ(1U, drag_start_count);
    EXPECT_EQ(2U, drag_count);
    EXPECT_EQ(1U, drag_end_count);
}

TEST_F(MainHandlerTest, Reset) {
    MainHandler mh(false);  // No VR.
    InitHandler(mh);
    EXPECT_TRUE(mh.HandleEvent(GetWidgetEvent(true)));
    EXPECT_FALSE(mh.IsWaiting());
    mh.Reset();
    EXPECT_TRUE(mh.IsWaiting());
}
