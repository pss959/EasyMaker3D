#include "Base/Event.h"
#include "Enums/Hand.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/WindowCamera.h"
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
    /// Sets up the scene in a PinchTracker.
    void InitForActivation(PinchTracker &pt);

    /// Returns an Event for a pinch press or release for the controller
    /// associated with the PinchTracker.
    static Event GetEvent(const PinchTracker &pt, bool is_press);

    /// Returns an Event that will intersect the left or right GenericWidget.
    static Event GetWidgetEvent(const PinchTracker &pt);
};

void PinchTrackerTest::InitForActivation(PinchTracker &pt) {
    // Set up a scene with GenericWidgets.
    InitTrackerScene(pt);
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
    EXPECT_EQ(Actuator::kLeftPinch,           lpt.GetActuator());
    EXPECT_EQ(Event::Device::kLeftController, lpt.GetDevice());
    EXPECT_EQ(TK::kPinchClickTimeout,         lpt.GetClickTimeout());

    PinchTracker rpt(Actuator::kRightPinch);
    EXPECT_EQ(Actuator::kRightPinch,           rpt.GetActuator());
    EXPECT_EQ(Event::Device::kRightController, rpt.GetDevice());
    EXPECT_EQ(TK::kPinchClickTimeout,         rpt.GetClickTimeout());
}

TEST_F(PinchTrackerTest, IsActivation) {
    PinchTracker lpt(Actuator::kLeftPinch);
    PinchTracker rpt(Actuator::kRightPinch);
    WidgetPtr    lwidget, rwidget;

    InitForActivation(lpt);
    InitForActivation(rpt);

    EXPECT_TRUE(lpt.IsActivation(GetEvent(lpt, true), lwidget));
    EXPECT_TRUE(rpt.IsActivation(GetEvent(rpt, true), rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong device.
    Event levent = GetEvent(lpt, true);
    Event revent = GetEvent(rpt, true);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lpt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong button.
    levent = GetEvent(lpt, true);
    revent = GetEvent(rpt, true);
    levent.button = Event::Button::kGrip;
    revent.button = Event::Button::kGrip;
    EXPECT_FALSE(lpt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong hand.
    levent = GetEvent(rpt, true);
    revent = GetEvent(lpt, true);
    EXPECT_FALSE(lpt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Not a press.
    levent = GetEvent(lpt, false);
    revent = GetEvent(rpt, false);
    EXPECT_FALSE(lpt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);
}

TEST_F(PinchTrackerTest, IsDeactivation) {
    PinchTracker lpt(Actuator::kLeftPinch);
    PinchTracker rpt(Actuator::kRightPinch);
    WidgetPtr    lwidget, rwidget;

    InitForActivation(lpt);
    InitForActivation(rpt);

    EXPECT_TRUE(lpt.IsDeactivation(GetEvent(lpt, false), lwidget));
    EXPECT_TRUE(rpt.IsDeactivation(GetEvent(rpt, false), rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong device.
    Event levent = GetEvent(lpt, false);
    Event revent = GetEvent(rpt, false);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lpt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong button.
    levent = GetEvent(lpt, false);
    revent = GetEvent(rpt, false);
    levent.button = Event::Button::kGrip;
    revent.button = Event::Button::kGrip;
    EXPECT_FALSE(lpt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong hand.
    levent = GetEvent(rpt, false);
    revent = GetEvent(lpt, false);
    EXPECT_FALSE(lpt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Not a press.
    levent = GetEvent(lpt, true);
    revent = GetEvent(rpt, true);
    EXPECT_FALSE(lpt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);
}

TEST_F(PinchTrackerTest, ActivationWidget) {
    PinchTracker lpt(Actuator::kLeftPinch);
    PinchTracker rpt(Actuator::kRightPinch);
    InitForActivation(lpt);
    InitForActivation(rpt);

    // Set up events that will intersect the left and right GenericWidgets.
    Event levent = GetWidgetEvent(lpt);
    Event revent = GetWidgetEvent(rpt);

    auto lw = GetLeftWidget();
    auto rw = GetRightWidget();

    WidgetPtr lwidget, rwidget;
    EXPECT_TRUE(lpt.IsActivation(levent, lwidget));
    EXPECT_TRUE(rpt.IsActivation(revent, rwidget));
    EXPECT_EQ(lw, lwidget);
    EXPECT_EQ(rw, rwidget);
}

TEST_F(PinchTrackerTest, Hover) {
    PinchTracker lpt(Actuator::kLeftPinch);
    PinchTracker rpt(Actuator::kRightPinch);
    InitForActivation(lpt);
    InitForActivation(rpt);

    // Set up events that will intersect the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lpt);
    Event revent = GetWidgetEvent(rpt);

    auto lw = GetLeftWidget();
    auto rw = GetRightWidget();

    EXPECT_FALSE(lw->IsHovering());
    EXPECT_FALSE(rw->IsHovering());
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_TRUE(lw->IsHovering());
    EXPECT_TRUE(rw->IsHovering());

    // Move off the GenericWidgets and back.
    levent.orientation = BuildRotation(0, 1, 0,  20);
    revent.orientation = BuildRotation(0, 1, 0, -20);
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_FALSE(lw->IsHovering());
    EXPECT_FALSE(rw->IsHovering());
    levent.orientation = BuildRotation(0, 1, 0,  1);
    revent.orientation = BuildRotation(0, 1, 0, -1);
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_TRUE(lw->IsHovering());
    EXPECT_TRUE(rw->IsHovering());

    lpt.StopHovering();
    rpt.StopHovering();
    EXPECT_FALSE(lw->IsHovering());
    EXPECT_FALSE(rw->IsHovering());

    // Cannot hover a disabled Widget.
    lw->SetInteractionEnabled(false);
    rw->SetInteractionEnabled(false);
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_FALSE(lw->IsHovering());
    EXPECT_FALSE(rw->IsHovering());
}

TEST_F(PinchTrackerTest, ClickDrag) {
    PinchTracker lpt(Actuator::kLeftPinch);
    PinchTracker rpt(Actuator::kRightPinch);
    InitForActivation(lpt);
    InitForActivation(rpt);

    // Set up events that will intersect the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lpt);
    Event revent = GetWidgetEvent(rpt);

    auto lw = GetLeftWidget();
    auto rw = GetRightWidget();

    // Activate to set the activation info.
    WidgetPtr lwidget, rwidget;
    EXPECT_TRUE(lpt.IsActivation(levent, lwidget));
    EXPECT_TRUE(rpt.IsActivation(revent, rwidget));

    // Update ClickInfos.
    ClickInfo lcinfo, rcinfo;
    lpt.FillClickInfo(lcinfo);
    rpt.FillClickInfo(rcinfo);
    EXPECT_EQ(Event::Device::kLeftController,  lcinfo.device);
    EXPECT_EQ(Event::Device::kRightController, rcinfo.device);
    EXPECT_EQ(lw.get(),                        lcinfo.widget);
    EXPECT_EQ(rw.get(),                        rcinfo.widget);

    // Update DragInfos.
    DragInfo ldinfo, rdinfo;
    lpt.FillActivationDragInfo(ldinfo);
    rpt.FillActivationDragInfo(rdinfo);
    EXPECT_EQ(Trigger::kPointer, ldinfo.trigger);
    EXPECT_EQ(Trigger::kPointer, rdinfo.trigger);
    EXPECT_EQ(lw,                ldinfo.hit.path.back());
    EXPECT_EQ(rw,                rdinfo.hit.path.back());

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
    EXPECT_EQ(Trigger::kPointer,  ldinfo.trigger);
    EXPECT_EQ(Trigger::kPointer,  rdinfo.trigger);
    EXPECT_FALSE(ldinfo.hit.IsValid());
    EXPECT_FALSE(rdinfo.hit.IsValid());

    lpt.Reset();
    rpt.Reset();
}
