#include "Base/Event.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/WindowCamera.h"
#include "Tests/Testing2.h"
#include "Tests/Trackers/TrackerTestBase.h"
#include "Trackers/MouseTracker.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"

// ----------------------------------------------------------------------------
// MouseTrackerTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class MouseTrackerTest : public TrackerTestBase {
  protected:
    MouseTracker     mt;   ///< MouseTracker

    GenericWidgetPtr lgw;  ///< Left GenericWidget.
    GenericWidgetPtr rgw;  ///< Right GenericWidget.

    WidgetPtr        tw;   ///< Widget returned by tracker.

    /// The constructor sets up the MouseTracker with a scene.
    MouseTrackerTest();

    /// Returns an Event for a mouse press or release.
    static Event GetEvent(bool is_press);
};

MouseTrackerTest::MouseTrackerTest() : mt(Actuator::kMouse) {
    // Set up a scene with GenericWidgets.
    InitTrackerScene(mt);

    // Set up the frustum in the MouseTracker.
    FrustumPtr frustum(new Frustum);
    GetWindowCamera()->BuildFrustum(Vector2i(100, 100), *frustum);
    mt.SetFrustum(frustum);

    // Access the GenericWidget pointers.
    lgw = GetLeftWidget();
    rgw = GetRightWidget();
}

Event MouseTrackerTest::GetEvent(bool is_press) {
    Event event;
    event.device = Event::Device::kMouse;
    event.button = Event::Button::kMouse1;
    event.flags.Set(
        is_press ? Event::Flag::kButtonPress : Event::Flag::kButtonRelease);
    return event;
}

// ----------------------------------------------------------------------------
// MouseTrackerTest Tests. Also tests PointerTracker.
// ----------------------------------------------------------------------------

TEST_F(MouseTrackerTest, Defaults) {
    // Constructing with the wrong Actuator should assert.
    TEST_ASSERT(MouseTracker(Actuator::kLeftPinch), "Actuator::kMouse");

    MouseTracker mt(Actuator::kMouse);
    EXPECT_ENUM_EQ(Actuator::kMouse,      mt.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kMouse, mt.GetDevice());
    EXPECT_EQ(TK::kMouseClickTimeout,     mt.GetClickTimeout());
}

TEST_F(MouseTrackerTest, IsActivation) {
    EXPECT_TRUE(mt.IsActivation(GetEvent(true), tw));
    EXPECT_NULL(tw);

    // Wrong device.
    Event event = GetEvent(true);
    event.device = Event::Device::kHeadset;
    EXPECT_FALSE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);

    // Wrong button.
    event = GetEvent(true);
    event.button = Event::Button::kMouse2;
    EXPECT_FALSE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);

    // Not a press.
    event = GetEvent(false);
    EXPECT_FALSE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);
}

TEST_F(MouseTrackerTest, IsDeactivation) {
    EXPECT_TRUE(mt.IsDeactivation(GetEvent(false), tw));
    EXPECT_NULL(tw);

    // Wrong device.
    Event event = GetEvent(false);
    event.device = Event::Device::kHeadset;
    EXPECT_FALSE(mt.IsDeactivation(event, tw));
    EXPECT_NULL(tw);

    // Wrong button.
    event = GetEvent(false);
    event.button = Event::Button::kMouse2;
    EXPECT_FALSE(mt.IsDeactivation(event, tw));
    EXPECT_NULL(tw);

    // Not a press.
    event = GetEvent(true);
    EXPECT_FALSE(mt.IsDeactivation(event, tw));
    EXPECT_NULL(tw);
}

TEST_F(MouseTrackerTest, ActivationWidget) {
    // Set up an event that will intersect the left GenericWidget.
    Event event = GetEvent(true);
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(.3f, .5f);

    WidgetPtr widget;
    EXPECT_TRUE(mt.IsActivation(event, tw));
    EXPECT_EQ(lgw, tw);
}

TEST_F(MouseTrackerTest, Hover) {
    // Set up an event that will intersect the left GenericWidget.
    Event event = GetEvent(true);
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(.5f, .5f);

    EXPECT_FALSE(lgw->IsHovering());
    mt.UpdateHovering(event);
    EXPECT_TRUE(lgw->IsHovering());

    // Install a path filter that ignores the GenericWidgets.
    auto filter = [](const SG::NodePath &){ return false; };
    mt.SetPathFilter(filter);
    mt.UpdateHovering(event);
    EXPECT_FALSE(lgw->IsHovering());
    mt.SetPathFilter(nullptr);
    mt.UpdateHovering(event);
    EXPECT_TRUE(lgw->IsHovering());

    mt.StopHovering();
    EXPECT_FALSE(lgw->IsHovering());

    // Cannot hover a disabled Widget.
    lgw->SetInteractionEnabled(false);
    mt.UpdateHovering(event);
    EXPECT_FALSE(lgw->IsHovering());
}

TEST_F(MouseTrackerTest, ClickDrag) {
    // Activate to set the activation info.
    Event event = GetEvent(true);
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(.6f, .5f);
    EXPECT_TRUE(mt.IsActivation(event, tw));

    // Update a ClickInfo.
    ClickInfo cinfo;
    mt.FillClickInfo(cinfo);
    EXPECT_ENUM_EQ(Event::Device::kMouse, cinfo.device);
    EXPECT_EQ(rgw.get(),                  cinfo.widget);

    // Update a DragInfo.
    DragInfo dinfo;
    mt.FillActivationDragInfo(dinfo);
    EXPECT_ENUM_EQ(Trigger::kPointer, dinfo.trigger);
    EXPECT_EQ(rgw,                    dinfo.hit.path.back());

    // Not a drag if no position.
    event.flags.Reset(Event::Flag::kPosition2D);
    EXPECT_FALSE(mt.MovedEnoughForDrag(event));
    event.flags.Set(Event::Flag::kPosition2D);

    // Not a drag if not far enough.
    event.position2D.Set(.6001f, .5f);
    EXPECT_FALSE(mt.MovedEnoughForDrag(event));

    // Drag if far enough.
    event.position2D.Set(.7f, .5f);
    EXPECT_TRUE(mt.MovedEnoughForDrag(event));

    // Update the DragInfo.
    mt.FillEventDragInfo(event, dinfo);
    EXPECT_ENUM_EQ(Trigger::kPointer, dinfo.trigger);
    EXPECT_EQ(rgw,                    dinfo.hit.path.back());

    mt.Reset();
}
