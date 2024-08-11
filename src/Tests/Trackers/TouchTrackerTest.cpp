//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/Event.h"
#include "Enums/Hand.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "Tests/Testing2.h"
#include "Tests/Trackers/TrackerTestBase.h"
#include "Tests/Trackers/TestTouchable.h"
#include "Trackers/TouchTracker.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/ITouchable.h"

// ----------------------------------------------------------------------------
// TouchTrackerTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class TouchTrackerTest : public TrackerTestBase {
  protected:
    TouchTracker     lt;   ///< TouchTracker for left controller.
    TouchTracker     rt;   ///< TouchTracker for right controller.

    TestTouchablePtr ltt;  ///< TestTouchable for left controller.
    TestTouchablePtr rtt;  ///< TestTouchable for right controller.

    GenericWidgetPtr lgw;  ///< Left GenericWidget.
    GenericWidgetPtr rgw;  ///< Right GenericWidget.

    WidgetPtr        ltw;  ///< Widget returned by left tracker.
    WidgetPtr        rtw;  ///< Widget returned by right tracker.

    /// The constructor sets up the left and right TouchTracker instances with
    /// a scene and TestTouchable instances.
    TouchTrackerTest();

    /// Returns an Event for a touch for the controller associated with the
    /// TouchTracker at the position (0, 0, \p z).
    static Event GetEvent(const TouchTracker &ttr, float z = 0);
};

TouchTrackerTest::TouchTrackerTest() : lt(Actuator::kLeftTouch),
                                       rt(Actuator::kRightTouch) {
    // Set up a scene with GenericWidgets.
    InitTrackerScene(lt);
    InitTrackerScene(rt);

    // Access the GenericWidget pointers.
    lgw = GetLeftWidget();
    rgw = GetRightWidget();

    // Set up the TestTouchable instances and set them in the TouchTrackers.
    ltt.reset(new TestTouchable);
    rtt.reset(new TestTouchable);
    ltt->widget = lgw;
    rtt->widget = rgw;
    lt.SetTouchable(ltt);
    rt.SetTouchable(rtt);
}

Event TouchTrackerTest::GetEvent(const TouchTracker &ttr, float z) {
    const bool is_left = ttr.GetActuator() == Actuator::kLeftTouch;

    Event event;
    event.device = is_left ? Event::Device::kLeftController :
        Event::Device::kRightController;
    event.flags.Set(Event::Flag::kTouch);
    event.touch_position3D.Set(0, 0, z);

    return event;
}

// ----------------------------------------------------------------------------
// TouchTrackerTest Tests.
// ----------------------------------------------------------------------------

TEST_F(TouchTrackerTest, Defaults) {
    // Constructing with the wrong Actuator should assert.
    TEST_ASSERT(TouchTracker(Actuator::kMouse), "actuator");

    TouchTracker lttr(Actuator::kLeftTouch);
    EXPECT_ENUM_EQ(Actuator::kLeftTouch,            lttr.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kLeftController,  lttr.GetDevice());
    EXPECT_EQ(0,                                    lttr.GetClickTimeout());

    TouchTracker rttr(Actuator::kRightTouch);
    EXPECT_ENUM_EQ(Actuator::kRightTouch,           rttr.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kRightController, rttr.GetDevice());
    EXPECT_EQ(0,                                    rttr.GetClickTimeout());
}

TEST_F(TouchTrackerTest, IsActivation) {
    EXPECT_TRUE(lt.IsActivation(GetEvent(lt), ltw));
    EXPECT_TRUE(rt.IsActivation(GetEvent(rt), rtw));
    EXPECT_EQ(lgw, ltw);
    EXPECT_EQ(rgw, rtw);

    // No touch flag.
    Event levent = GetEvent(lt);
    Event revent = GetEvent(rt);
    levent.flags.Reset(Event::Flag::kTouch);
    revent.flags.Reset(Event::Flag::kTouch);
    EXPECT_FALSE(lt.IsActivation(levent, ltw));
    EXPECT_FALSE(rt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong device.
    levent = GetEvent(lt);
    revent = GetEvent(rt);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lt.IsActivation(levent, ltw));
    EXPECT_FALSE(rt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rt);
    revent = GetEvent(lt);
    EXPECT_FALSE(lt.IsActivation(levent, ltw));
    EXPECT_FALSE(rt.IsActivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(TouchTrackerTest, IsDeactivation) {
    // IsDeactivation() requires that the TouchTracker was activated.
    EXPECT_TRUE(lt.IsActivation(GetEvent(lt), ltw));
    EXPECT_TRUE(rt.IsActivation(GetEvent(rt), rtw));

    // Deactivation requires changing the Z position enough to simulate a
    // touch AND hitting a different Widget or no Widget.

    // Not enough motion.
    EXPECT_FALSE(lt.IsDeactivation(GetEvent(lt, .01f), ltw));
    EXPECT_FALSE(rt.IsDeactivation(GetEvent(rt, .01f), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Enough motion, but hit same Widget.
    EXPECT_FALSE(lt.IsDeactivation(GetEvent(lt, 1), ltw));
    EXPECT_FALSE(rt.IsDeactivation(GetEvent(rt, 1), rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Enough motion, hit no widget => deactivate and return touched Widget.
    ltt->widget.reset();
    rtt->widget.reset();
    EXPECT_TRUE(lt.IsDeactivation(GetEvent(lt, 10), ltw));
    EXPECT_TRUE(rt.IsDeactivation(GetEvent(rt, 10), rtw));
    EXPECT_EQ(lgw, ltw);
    EXPECT_EQ(rgw, rtw);

    // Activate again.
    ltt->widget = lgw;
    rtt->widget = rgw;
    EXPECT_TRUE(lt.IsActivation(GetEvent(lt), ltw));
    EXPECT_TRUE(rt.IsActivation(GetEvent(rt), rtw));

    // Enough motion, hit different widget.
    ltt->widget = rgw;
    rtt->widget = lgw;
    EXPECT_TRUE(lt.IsDeactivation(GetEvent(lt, 10), ltw));
    EXPECT_TRUE(rt.IsDeactivation(GetEvent(rt, 10), rtw));
    EXPECT_EQ(rgw, ltw);
    EXPECT_EQ(lgw, rtw);

    // Activate again.
    ltt->widget = lgw;
    rtt->widget = rgw;
    EXPECT_TRUE(lt.IsActivation(GetEvent(lt), ltw));
    EXPECT_TRUE(rt.IsActivation(GetEvent(rt), rtw));

    // No touch flag.
    Event levent = GetEvent(lt);
    Event revent = GetEvent(rt);
    levent.flags.Reset(Event::Flag::kTouch);
    revent.flags.Reset(Event::Flag::kTouch);
    EXPECT_FALSE(lt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong device.
    levent = GetEvent(lt);
    revent = GetEvent(rt);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);

    // Wrong hand.
    levent = GetEvent(rt);
    revent = GetEvent(lt);
    EXPECT_FALSE(lt.IsDeactivation(levent, ltw));
    EXPECT_FALSE(rt.IsDeactivation(revent, rtw));
    EXPECT_NULL(ltw);
    EXPECT_NULL(rtw);
}

TEST_F(TouchTrackerTest, Hover) {
    // TouchTracker never hovers.
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    lt.UpdateHovering(GetEvent(lt));
    rt.UpdateHovering(GetEvent(rt));
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
    lt.StopHovering();
    rt.StopHovering();
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
}

TEST_F(TouchTrackerTest, ClickDrag) {
    // Activate to set the activation info.
    Event levent = GetEvent(lt, 2);
    Event revent = GetEvent(rt, 2);
    EXPECT_TRUE(lt.IsActivation(levent, ltw));
    EXPECT_TRUE(rt.IsActivation(revent, rtw));

    // Update ClickInfos.
    ClickInfo lcinfo, rcinfo;
    lt.FillClickInfo(lcinfo);
    rt.FillClickInfo(rcinfo);
    EXPECT_ENUM_EQ(Event::Device::kLeftController,  lcinfo.device);
    EXPECT_ENUM_EQ(Event::Device::kRightController, rcinfo.device);
    EXPECT_EQ(lgw.get(),                            lcinfo.widget);
    EXPECT_EQ(rgw.get(),                            rcinfo.widget);

    // Update DragInfos.
    DragInfo ldinfo, rdinfo;
    lt.FillActivationDragInfo(ldinfo);
    rt.FillActivationDragInfo(rdinfo);
    EXPECT_ENUM_EQ(Trigger::kTouch, ldinfo.trigger);
    EXPECT_ENUM_EQ(Trigger::kTouch, rdinfo.trigger);
    EXPECT_FALSE(ldinfo.hit.IsValid());
    EXPECT_FALSE(rdinfo.hit.IsValid());
    EXPECT_EQ(Point3f(0, 0, 2), ldinfo.touch_position);
    EXPECT_EQ(Point3f(0, 0, 2), rdinfo.touch_position);

    // Not a drag if not a touch.
    levent.flags.Reset(Event::Flag::kTouch);
    revent.flags.Reset(Event::Flag::kTouch);
    EXPECT_FALSE(lt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rt.MovedEnoughForDrag(revent));
    levent.flags.Set(Event::Flag::kTouch);
    revent.flags.Set(Event::Flag::kTouch);

    // Not a drag if not far enough.
    levent.touch_position3D[0] = .001f;
    revent.touch_position3D[0] = .001f;
    EXPECT_FALSE(lt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rt.MovedEnoughForDrag(revent));

    // Drag requires more motion if there is any click observer on the Widget.
    auto click = [](const ClickInfo &){};
    const float x = 1.2f * TK::kMinTouchControllerDistance;
    lgw->GetClicked().AddObserver("key", click);
    rgw->GetClicked().AddObserver("key", click);
    levent.touch_position3D[0] = x;
    revent.touch_position3D[0] = x;
    EXPECT_FALSE(lt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rt.MovedEnoughForDrag(revent));

    // Without an observer, this is far enough.
    lgw->GetClicked().RemoveObserver("key");
    rgw->GetClicked().RemoveObserver("key");
    EXPECT_TRUE(lt.MovedEnoughForDrag(levent));
    EXPECT_TRUE(rt.MovedEnoughForDrag(revent));

    // Update the DragInfo.
    lt.FillEventDragInfo(levent, ldinfo);
    rt.FillEventDragInfo(revent, rdinfo);
    EXPECT_ENUM_EQ(Trigger::kTouch, ldinfo.trigger);
    EXPECT_ENUM_EQ(Trigger::kTouch, rdinfo.trigger);
    EXPECT_EQ(Point3f(x, 0, 2),     ldinfo.touch_position);
    EXPECT_EQ(Point3f(x, 0, 2),     rdinfo.touch_position);

    lt.Reset();
    rt.Reset();
}
