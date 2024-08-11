//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/Event.h"
#include "Enums/Hand.h"
#include "Items/Grippable.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "Tests/Testing2.h"
#include "Tests/Trackers/TrackerTestBase.h"
#include "Tests/Trackers/TestGrippable.h"
#include "Trackers/GripTracker.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"

/// \ingroup Tests
class GripTrackerTest : public TrackerTestBase {
  protected:
    GripTracker      lgt;  ///< GripTracker for left controller.
    GripTracker      rgt;  ///< GripTracker for right controller.

    TestGrippablePtr ltg;  ///< TestGrippable for left controller.
    TestGrippablePtr rtg;  ///< TestGrippable for right controller.

    GenericWidgetPtr lgw;  ///< Left GenericWidget.
    GenericWidgetPtr rgw;  ///< Right GenericWidget.

    WidgetPtr        ltw;  ///< Widget returned by left tracker.
    WidgetPtr        rtw;  ///< Widget returned by right tracker.

    /// The constructor sets up the left and right GripTracker instances with
    /// a scene and TestGrippable instances.
    GripTrackerTest();

    /// Returns an Event for a grip press or release for the controller
    /// associated with the GripTracker.
    static Event GetEvent(const GripTracker &gt, bool is_press);

    /// Returns an Event that will intersect the left or right GenericWidget.
    static Event GetWidgetEvent(const GripTracker &gt);
};

GripTrackerTest::GripTrackerTest() : lgt(Actuator::kLeftGrip),
                                     rgt(Actuator::kRightGrip) {
    Parser::Registry::AddType<TestGrippable>("TestGrippable");

    // Set up a scene with GenericWidgets.
    InitTrackerScene(lgt);
    InitTrackerScene(rgt);

    // Access the GenericWidget pointers.
    lgw = GetLeftWidget();
    rgw = GetRightWidget();

    // Set up the TestGrippable instances and set them in the GripTrackers.
    ltg = CreateObject<TestGrippable>();
    rtg = CreateObject<TestGrippable>();
    ltg->widget = lgw;
    ltg->target = Point3f(-1, 0, 0);
    ltg->color  = Color(1, 0, 0);
    rtg->widget = rgw;
    rtg->target = Point3f( 1, 0, 0);
    rtg->color  = Color(0, 0, 1);
    lgt.SetGrippable(ltg, SG::NodePath(ltg));
    rgt.SetGrippable(rtg, SG::NodePath(rtg));
}

Event GripTrackerTest::GetEvent(const GripTracker &gt, bool is_press) {
    const bool is_left = gt.GetActuator() == Actuator::kLeftGrip;

    Event event;
    event.device = is_left ? Event::Device::kLeftController :
        Event::Device::kRightController;
    event.button = Event::Button::kGrip;
    event.flags.Set(
        is_press ? Event::Flag::kButtonPress : Event::Flag::kButtonRelease);
    return event;
}

Event GripTrackerTest::GetWidgetEvent(const GripTracker &gt) {
    // The orientation should align the guide direction with -Z.
    Event event = GetEvent(gt, true);
    event.flags.Set(Event::Flag::kPosition3D);
    event.flags.Set(Event::Flag::kOrientation);
    event.position3D.Set(0, 0, 10);
    event.orientation = BuildRotation(0, 1, 0, -90);
    return event;
}

// ----------------------------------------------------------------------------
// GripTrackerTest Tests.
// ----------------------------------------------------------------------------

TEST_F(GripTrackerTest, Defaults) {
    // Constructing with the wrong Actuator should assert.
    TEST_ASSERT(GripTracker(Actuator::kMouse), "actuator");

    GripTracker lgt(Actuator::kLeftGrip);
    EXPECT_ENUM_EQ(Actuator::kLeftGrip,             lgt.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kLeftController,  lgt.GetDevice());
    EXPECT_EQ(TK::kGripClickTimeout,                lgt.GetClickTimeout());

    GripTracker rgt(Actuator::kRightGrip);
    EXPECT_ENUM_EQ(Actuator::kRightGrip,            rgt.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kRightController, rgt.GetDevice());
    EXPECT_EQ(TK::kGripClickTimeout,                rgt.GetClickTimeout());
}

TEST_F(GripTrackerTest, IsActivation) {
    EXPECT_TRUE(lgt.IsActivation(GetEvent(lgt, true), ltw));
    EXPECT_TRUE(rgt.IsActivation(GetEvent(rgt, true), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong device.
    Event levent = GetEvent(lgt, true);
    Event revent = GetEvent(rgt, true);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lgt.IsActivation(levent, ltw));
    EXPECT_FALSE(rgt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong button.
    levent = GetEvent(lgt, true);
    revent = GetEvent(rgt, true);
    levent.button = Event::Button::kPinch;
    revent.button = Event::Button::kPinch;
    EXPECT_FALSE(lgt.IsActivation(levent, ltw));
    EXPECT_FALSE(rgt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rgt, true);
    revent = GetEvent(lgt, true);
    EXPECT_FALSE(lgt.IsActivation(levent, ltw));
    EXPECT_FALSE(rgt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Not a press.
    levent = GetEvent(lgt, false);
    revent = GetEvent(rgt, false);
    EXPECT_FALSE(lgt.IsActivation(levent, ltw));
    EXPECT_FALSE(rgt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(GripTrackerTest, IsDeactivation) {
    EXPECT_TRUE(lgt.IsDeactivation(GetEvent(lgt, false), ltw));
    EXPECT_TRUE(rgt.IsDeactivation(GetEvent(rgt, false), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong device.
    Event levent = GetEvent(lgt, false);
    Event revent = GetEvent(rgt, false);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lgt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong button.
    levent = GetEvent(lgt, false);
    revent = GetEvent(rgt, false);
    levent.button = Event::Button::kPinch;
    revent.button = Event::Button::kPinch;
    EXPECT_FALSE(lgt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rgt, false);
    revent = GetEvent(lgt, false);
    EXPECT_FALSE(lgt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Not a press.
    levent = GetEvent(lgt, true);
    revent = GetEvent(rgt, true);
    EXPECT_FALSE(lgt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(GripTrackerTest, ActivationWidget) {
    // Set up events that will intersect the left and right GenericWidgets.
    Event levent = GetWidgetEvent(lgt);
    Event revent = GetWidgetEvent(rgt);

    EXPECT_TRUE(lgt.IsActivation(levent, ltw));
    EXPECT_TRUE(rgt.IsActivation(revent, rtw));
    EXPECT_EQ(lgw, ltw);
    EXPECT_EQ(rgw, rtw);
}

TEST_F(GripTrackerTest, Hover) {
    // Set up events that will return the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lgt);
    Event revent = GetWidgetEvent(rgt);

    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    lgt.UpdateHovering(levent);
    rgt.UpdateHovering(revent);
    EXPECT_TRUE(lgw->IsHovering());
    EXPECT_TRUE(rgw->IsHovering());

    // This causes each TestGrippable to return a null widget.
    ltg->widget.reset();
    rtg->widget.reset();
    lgt.UpdateHovering(levent);
    rgt.UpdateHovering(revent);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());

    // Restore the real widgets.
    ltg->widget = lgw;
    rtg->widget = rgw;
    lgt.UpdateHovering(levent);
    rgt.UpdateHovering(revent);
    EXPECT_TRUE(lgw->IsHovering());
    EXPECT_TRUE(rgw->IsHovering());

    lgt.StopHovering();
    rgt.StopHovering();
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());

    // Test that activation stops hovering.
    lgt.UpdateHovering(levent);
    rgt.UpdateHovering(revent);
    EXPECT_TRUE(lgw->IsHovering());
    EXPECT_TRUE(rgw->IsHovering());
    WidgetPtr ltw, rtw;
    EXPECT_TRUE(lgt.IsActivation(levent, ltw));
    EXPECT_TRUE(rgt.IsActivation(revent, rtw));
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    EXPECT_TRUE(lgt.IsDeactivation(GetEvent(lgt, false), ltw));
    EXPECT_TRUE(rgt.IsDeactivation(GetEvent(rgt, false), rtw));

    // Cannot hover a disabled Widget.
    lgw->SetInteractionEnabled(false);
    rgw->SetInteractionEnabled(false);
    lgt.UpdateHovering(levent);
    rgt.UpdateHovering(revent);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
}

TEST_F(GripTrackerTest, ClickDrag) {
    // Set up events that will intersect the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lgt);
    Event revent = GetWidgetEvent(rgt);

    // Activate to set the activation info.
    EXPECT_TRUE(lgt.IsActivation(levent, ltw));
    EXPECT_TRUE(rgt.IsActivation(revent, rtw));

    // Update ClickInfos.
    ClickInfo lcinfo, rcinfo;
    lgt.FillClickInfo(lcinfo);
    rgt.FillClickInfo(rcinfo);
    EXPECT_ENUM_EQ(Event::Device::kLeftController,  lcinfo.device);
    EXPECT_ENUM_EQ(Event::Device::kRightController, rcinfo.device);
    EXPECT_EQ(lgw.get(),                            lcinfo.widget);
    EXPECT_EQ(rgw.get(),                            rcinfo.widget);

    // Update DragInfos.
    DragInfo ldinfo, rdinfo;
    lgt.FillActivationDragInfo(ldinfo);
    rgt.FillActivationDragInfo(rdinfo);
    EXPECT_ENUM_EQ(Trigger::kGrip, ldinfo.trigger);
    EXPECT_ENUM_EQ(Trigger::kGrip, rdinfo.trigger);
    EXPECT_FALSE(ldinfo.hit.IsValid());
    EXPECT_FALSE(rdinfo.hit.IsValid());
    EXPECT_EQ(Vector3f(-1, 0, 0),          ldinfo.grip_guide_direction);
    EXPECT_EQ(Vector3f(-1, 0, 0),          rdinfo.grip_guide_direction);
    EXPECT_EQ(Point3f(0, 0, 10),           ldinfo.grip_position);
    EXPECT_EQ(Point3f(0, 0, 10),           rdinfo.grip_position);
    EXPECT_EQ(BuildRotation(0, 1, 0, -90), ldinfo.grip_orientation);
    EXPECT_EQ(BuildRotation(0, 1, 0, -90), rdinfo.grip_orientation);

    // Not a drag if no position.
    levent.flags.Reset(Event::Flag::kPosition3D);
    revent.flags.Reset(Event::Flag::kPosition3D);
    EXPECT_FALSE(lgt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rgt.MovedEnoughForDrag(revent));
    levent.flags.Set(Event::Flag::kPosition3D);
    revent.flags.Set(Event::Flag::kPosition3D);

    // Not a drag if not far enough.
    levent.orientation = BuildRotation(0, 1, 0, -91);
    revent.orientation = BuildRotation(0, 1, 0, -92);
    EXPECT_FALSE(lgt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rgt.MovedEnoughForDrag(revent));

    // Drag if far enough. (Left changes rotation, right changes position.)
    levent.orientation = BuildRotation(0, 1, 0, -110);
    revent.position3D  = revent.position3D + Vector3f(.1f, 0, 0);
    EXPECT_TRUE(lgt.MovedEnoughForDrag(levent));
    EXPECT_TRUE(rgt.MovedEnoughForDrag(revent));

    // Update the DragInfo.
    lgt.FillEventDragInfo(levent, ldinfo);
    rgt.FillEventDragInfo(revent, rdinfo);
    EXPECT_ENUM_EQ(Trigger::kGrip,          ldinfo.trigger);
    EXPECT_ENUM_EQ(Trigger::kGrip,          rdinfo.trigger);
    EXPECT_EQ(Vector3f(-1, 0, 0),           ldinfo.grip_guide_direction);
    EXPECT_EQ(Vector3f(-1, 0, 0),           rdinfo.grip_guide_direction);
    EXPECT_EQ(Point3f(0,   0, 10),          ldinfo.grip_position);
    EXPECT_EQ(Point3f(.1f, 0, 10),          rdinfo.grip_position);
    EXPECT_EQ(BuildRotation(0, 1, 0, -110), ldinfo.grip_orientation);
    EXPECT_EQ(BuildRotation(0, 1, 0,  -92), rdinfo.grip_orientation);

    lgt.Reset();
    rgt.Reset();
}
