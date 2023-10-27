#include "Base/Event.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "Tests/Testing2.h"
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

    /// Returns an Event for a controller pinch press, release, or motion. \p
    /// action is "press", "release", or empty for neither. \p widget is "L" to
    /// position over the left GenericWidget), "R" for the right GenericWidget,
    /// or empty for neither.
    static Event GetEvent(const PinchTracker &pt,
                          const Str &action, const Str &widget);
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

Event PinchTrackerTest::GetEvent(const PinchTracker &pt,
                                 const Str &action, const Str &widget) {
    const bool is_left = pt.GetActuator() == Actuator::kLeftPinch;

    Event event;
    event.device = is_left ? Event::Device::kLeftController :
        Event::Device::kRightController;

    if (action == "press" || action == "release") {
        event.button = Event::Button::kPinch;
        event.flags.Set(action == "press" ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
    }

    event.flags.Set(Event::Flag::kPosition3D);
    event.flags.Set(Event::Flag::kOrientation);

    event.position3D.Set(0, 0, 10);

    // Set the orientation to hit the target GenericWidget.
    if (widget == "L")
        event.orientation = BuildRotation(0, 1, 0, 10);
    else if (widget == "R")
        event.orientation = BuildRotation(0, 1, 0, -10);
    else
        event.orientation = BuildRotation(0, 1, 0, 90);

    return event;
}

// ----------------------------------------------------------------------------
// PinchTrackerTest Tests. Also tests PointerTracker.
// ----------------------------------------------------------------------------

TEST_F(PinchTrackerTest, Defaults) {
    // Constructing with the wrong Actuator should assert.
    TEST_ASSERT(PinchTracker(Actuator::kMouse), "actuator");

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
    // Activation with no Widget.
    EXPECT_TRUE(lpt.IsActivation(GetEvent(lpt, "press", ""), ltw));
    EXPECT_TRUE(rpt.IsActivation(GetEvent(rpt, "press", ""), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Activation with GenericWidget.
    EXPECT_TRUE(lpt.IsActivation(GetEvent(lpt, "press", "L"), ltw));
    EXPECT_TRUE(rpt.IsActivation(GetEvent(rpt, "press", "R"), rtw));
    EXPECT_EQ(lgw, ltw);
    EXPECT_EQ(rgw, rtw);

    // Wrong device.
    Event levent = GetEvent(lpt, "press", "");
    Event revent = GetEvent(rpt, "press", "");
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong button.
    levent = GetEvent(lpt, "press", "");
    revent = GetEvent(rpt, "press", "");
    levent.button = Event::Button::kGrip;
    revent.button = Event::Button::kGrip;
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rpt, "press", "");
    revent = GetEvent(lpt, "press", "");
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Not a press.
    levent = GetEvent(lpt, "release", "");
    revent = GetEvent(rpt, "release", "");
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
    levent = GetEvent(lpt, "", "");
    revent = GetEvent(rpt, "", "");
    EXPECT_FALSE(lpt.IsActivation(levent, ltw));
    EXPECT_FALSE(rpt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(PinchTrackerTest, IsDeactivation) {
    // Deactivation with no Widget.
    EXPECT_TRUE(lpt.IsDeactivation(GetEvent(lpt, "release", ""), ltw));
    EXPECT_TRUE(rpt.IsDeactivation(GetEvent(rpt, "release", ""), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Deactivation with GenericWidget.
    EXPECT_TRUE(lpt.IsDeactivation(GetEvent(lpt, "release", "L"), ltw));
    EXPECT_TRUE(rpt.IsDeactivation(GetEvent(rpt, "release", "R"), rtw));
    EXPECT_EQ(lgw, ltw);
    EXPECT_EQ(rgw, rtw);

    // Wrong device.
    Event levent = GetEvent(lpt, "release", "");
    Event revent = GetEvent(rpt, "release", "");
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong button.
    levent = GetEvent(lpt, "release", "");
    revent = GetEvent(rpt, "release", "");
    levent.button = Event::Button::kGrip;
    revent.button = Event::Button::kGrip;
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rpt, "release", "");
    revent = GetEvent(lpt, "release", "");
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Not a release.
    levent = GetEvent(lpt, "press", "");
    revent = GetEvent(rpt, "press", "");
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
    levent = GetEvent(lpt, "", "");
    revent = GetEvent(rpt, "", "");
    EXPECT_FALSE(lpt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rpt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(PinchTrackerTest, Hover) {
    // Set up events that will intersect the left/right GenericWidgets.
    Event levent = GetEvent(lpt, "", "L");
    Event revent = GetEvent(rpt, "", "R");

    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_TRUE(lgw->IsHovering());
    EXPECT_TRUE(rgw->IsHovering());

    // Move off the GenericWidgets and onto the opposite one.
    levent = GetEvent(lpt, "", "");
    revent = GetEvent(rpt, "", "");
    lpt.UpdateHovering(levent);
    rpt.UpdateHovering(revent);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    levent = GetEvent(lpt, "", "R");
    revent = GetEvent(rpt, "", "L");
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
    // Activate the GenericWidgets to set the activation info.
    Event levent = GetEvent(lpt, "press", "L");
    Event revent = GetEvent(rpt, "press", "R");
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
    EXPECT_EQ(lgw,                    ldinfo.hit.path.back());
    EXPECT_EQ(rgw,                    rdinfo.hit.path.back());

    // Not a drag if no position.
    levent = GetEvent(lpt, "", "L");
    revent = GetEvent(rpt, "", "R");
    levent.flags.Reset(Event::Flag::kPosition3D);
    revent.flags.Reset(Event::Flag::kPosition3D);
    EXPECT_FALSE(lpt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rpt.MovedEnoughForDrag(revent));

    // Not a drag if rotation does not change enough.
    levent = GetEvent(lpt, "", "L");
    revent = GetEvent(rpt, "", "R");
    levent.orientation *= BuildRotation(0, 1, 0,  2);
    revent.orientation *= BuildRotation(0, 1, 0, -2);
    EXPECT_FALSE(lpt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rpt.MovedEnoughForDrag(revent));

    // Drag if far enough.
    levent.orientation *= BuildRotation(0, 1, 0,  10);
    revent.orientation *= BuildRotation(0, 1, 0, -10);
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
