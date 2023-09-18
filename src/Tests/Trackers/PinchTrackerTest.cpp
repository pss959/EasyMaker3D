#include "Base/Event.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "Tests/Testing.h"
#include "Tests/Trackers/TrackerTestBase.h"
#include "Trackers/PinchTracker.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"

// ----------------------------------------------------------------------------
// PinchTrackerTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class PinchTrackerTest : public TrackerTestBase {
  protected:
    PinchTracker     lpt;  ///< PinchTracker for left controller.
    PinchTracker     rpt;  ///< PinchTracker for right controller.

    GenericWidgetPtr lgw;  ///< Left GenericWidget.
    GenericWidgetPtr rgw;  ///< Right GenericWidget.

    WidgetPtr        ltw;  ///< Widget returned by left tracker.
    WidgetPtr        rtw;  ///< Widget returned by right tracker.

    /// The constructor sets up the left and right PinchTracker instances with
    /// a scene.
    PinchTrackerTest();

    /// Returns an Event for a pinch press or release for the controller
    /// associated with the PinchTracker.
    static Event GetEvent(const PinchTracker &pt, bool is_press);

    /// Returns an Event that will intersect the left or right GenericWidget.
    static Event GetWidgetEvent(const PinchTracker &pt);
};

PinchTrackerTest::PinchTrackerTest() : lpt(Actuator::kLeftPinch),
                                       rpt(Actuator::kRightPinch) {
    // Set up a scene with GenericWidgets.
    InitTrackerScene(lpt);
    InitTrackerScene(rpt);

    // Access the GenericWidget pointers.
    lgw = GetLeftWidget();
    rgw = GetRightWidget();
}

Event PinchTrackerTest::GetEvent(const PinchTracker &pt, bool is_press) {
    const bool is_left = pt.GetActuator() == Actuator::kLeftPinch;

    Event event;
    event.device = is_left ? Event::Device::kLeftController :
        Event::Device::kRightController;
    event.button = Event::Button::kPinch;
    event.flags.Set(
        is_press ? Event::Flag::kButtonPress : Event::Flag::kButtonRelease);
    return event;
}

Event PinchTrackerTest::GetWidgetEvent(const PinchTracker &pt) {
    const bool is_left = pt.GetActuator() == Actuator::kLeftPinch;

    Event event = GetEvent(pt, true);
    event.flags.Set(Event::Flag::kPosition3D);
    event.flags.Set(Event::Flag::kOrientation);
    event.position3D.Set(0, 0, 10);
    event.orientation = BuildRotation(0, 1, 0, is_left ? 10 : -10);
    return event;
}

// ----------------------------------------------------------------------------
// PinchTrackerTest Tests. Also tests PointerTracker.
// ----------------------------------------------------------------------------

TEST_F(PinchTrackerTest, Defaults) {
    // Constructing with the wrong Actuator should assert.
    TEST_THROW(PinchTracker(Actuator::kMouse), AssertException, "actuator");

    PinchTracker lpt(Actuator::kLeftPinch);
    EXPECT_ENUM_EQ(Actuator::kLeftPinch,           lpt.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kLeftController, lpt.GetDevice());
    EXPECT_EQ(TK::kPinchClickTimeout,              lpt.GetClickTimeout());

    PinchTracker rpt(Actuator::kRightPinch);
    EXPECT_ENUM_EQ(Actuator::kRightPinch,           rpt.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kRightController, rpt.GetDevice());
    EXPECT_EQ(TK::kPinchClickTimeout,               rpt.GetClickTimeout());
}

TEST_F(PinchTrackerTest, IsActivation) {
    EXPECT_TRUE(lpt.IsActivation(GetEvent(lpt, true), ltw));
    EXPECT_TRUE(rpt.IsActivation(GetEvent(rpt, true), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong device.
    Event levent = GetEvent(lpt, true);
    Event revent = GetEvent(rpt, true);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong button.
    levent = GetEvent(lpt, true);
    revent = GetEvent(rpt, true);
    levent.button = Event::Button::kGrip;
    revent.button = Event::Button::kGrip;
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rpt, true);
    revent = GetEvent(lpt, true);
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Not a press.
    levent = GetEvent(lpt, false);
    revent = GetEvent(rpt, false);
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(PinchTrackerTest, IsDeactivation) {
    EXPECT_TRUE(lpt.IsDeactivation(GetEvent(lpt, false), ltw));
    EXPECT_TRUE(rpt.IsDeactivation(GetEvent(rpt, false), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong device.
    Event levent = GetEvent(lpt, false);
    Event revent = GetEvent(rpt, false);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong button.
    levent = GetEvent(lpt, false);
    revent = GetEvent(rpt, false);
    levent.button = Event::Button::kGrip;
    revent.button = Event::Button::kGrip;
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rpt, false);
    revent = GetEvent(lpt, false);
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Not a press.
    levent = GetEvent(lpt, true);
    revent = GetEvent(rpt, true);
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(PinchTrackerTest, ActivationWidget) {
    // Set up events that will intersect the left and right GenericWidgets.
    Event levent = GetWidgetEvent(lpt);
    Event revent = GetWidgetEvent(rpt);

    EXPECT_TRUE(lpt.IsActivation(levent, ltw));
    EXPECT_TRUE(rpt.IsActivation(revent, rtw));
    EXPECT_EQ(lgw, ltw);
    EXPECT_EQ(rgw, rtw);
}

TEST_F(PinchTrackerTest, Hover) {
    // Set up events that will intersect the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lpt);
    Event revent = GetWidgetEvent(rpt);

    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_TRUE(lgw->IsHovering());
    EXPECT_TRUE(rgw->IsHovering());

    // Move off the GenericWidgets and back.
    levent.orientation = BuildRotation(0, 1, 0,  20);
    revent.orientation = BuildRotation(0, 1, 0, -20);
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    levent.orientation = BuildRotation(0, 1, 0,  1);
    revent.orientation = BuildRotation(0, 1, 0, -1);
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_TRUE(lgw->IsHovering());
    EXPECT_TRUE(rgw->IsHovering());

    lpt.StopHovering();
    rpt.StopHovering();
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());

    // Cannot hover a disabled Widget.
    lgw->SetInteractionEnabled(false);
    rgw->SetInteractionEnabled(false);
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
}

TEST_F(PinchTrackerTest, ClickDrag) {
    // Set up events that will intersect the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lpt);
    Event revent = GetWidgetEvent(rpt);

    // Activate to set the activation info.
    EXPECT_TRUE(lpt.IsActivation(levent, ltw));
    EXPECT_TRUE(rpt.IsActivation(revent, rtw));

    // Update ClickInfos.
    ClickInfo lcinfo, rcinfo;
    lpt.FillClickInfo(lcinfo);
    rpt.FillClickInfo(rcinfo);
    EXPECT_ENUM_EQ(Event::Device::kLeftController,  lcinfo.device);
    EXPECT_ENUM_EQ(Event::Device::kRightController, rcinfo.device);
    EXPECT_EQ(lgw.get(),                            lcinfo.widget);
    EXPECT_EQ(rgw.get(),                            rcinfo.widget);

    // Update DragInfos.
    DragInfo ldinfo, rdinfo;
    lpt.FillActivationDragInfo(ldinfo);
    rpt.FillActivationDragInfo(rdinfo);
    EXPECT_ENUM_EQ(Trigger::kPointer, ldinfo.trigger);
    EXPECT_ENUM_EQ(Trigger::kPointer, rdinfo.trigger);
    EXPECT_EQ(lgw,                     ldinfo.hit.path.back());
    EXPECT_EQ(rgw,                     rdinfo.hit.path.back());

    // Not a drag if no position.
    levent.flags.Reset(Event::Flag::kPosition3D);
    revent.flags.Reset(Event::Flag::kPosition3D);
    EXPECT_FALSE(lpt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rpt.MovedEnoughForDrag(revent));
    levent.flags.Set(Event::Flag::kPosition3D);
    revent.flags.Set(Event::Flag::kPosition3D);

    // Not a drag if not far enough.
    levent.orientation = BuildRotation(0, 1, 0,  11);
    revent.orientation = BuildRotation(0, 1, 0, -11);
    EXPECT_FALSE(lpt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rpt.MovedEnoughForDrag(revent));

    // Drag if far enough.
    levent.orientation = BuildRotation(0, 1, 0,  15);
    revent.orientation = BuildRotation(0, 1, 0, -15);
    EXPECT_TRUE(lpt.MovedEnoughForDrag(levent));
    EXPECT_TRUE(rpt.MovedEnoughForDrag(revent));

    // Update the DragInfo. Note that the angle change is enough to miss the
    // GenericWidgets.
    lpt.FillEventDragInfo(levent, ldinfo);
    rpt.FillEventDragInfo(revent, rdinfo);
    EXPECT_ENUM_EQ(Trigger::kPointer,  ldinfo.trigger);
    EXPECT_ENUM_EQ(Trigger::kPointer,  rdinfo.trigger);
    EXPECT_FALSE(ldinfo.hit.IsValid());
    EXPECT_FALSE(rdinfo.hit.IsValid());

    lpt.Reset();
    rpt.Reset();
}
