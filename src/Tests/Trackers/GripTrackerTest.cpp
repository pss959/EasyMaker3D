#include "Base/Event.h"
#include "Enums/Hand.h"
#include "Items/Grippable.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/WindowCamera.h"
#include "Tests/Testing.h"
#include "Tests/Trackers/TrackerTestBase.h"
#include "Trackers/GripTracker.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"

// ----------------------------------------------------------------------------
// GripTrackerTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class GripTrackerTest : public TrackerTestBase {
  protected:
    class TestGrippable;
    DECL_SHARED_PTR(TestGrippable);

    GripTrackerTest();

    /// Sets up the scene in a GripTracker.
    void InitForActivation(GripTracker &gt);

    /// Returns an Event for a grip press or release for the controller
    /// associated with the GripTracker.
    static Event GetEvent(const GripTracker &gt, bool is_press);

    /// Returns an Event that will intersect the left or right GenericWidget.
    static Event GetWidgetEvent(const GripTracker &gt);

  private:
    /// TestGrippable instance used to set up a GripInfo.
    TestGrippablePtr tg_;
};

// ----------------------------------------------------------------------------
// GripTrackerTest::TestGrippable class.
// ----------------------------------------------------------------------------

/// Derived Grippable class used for testing the GripTracker.
class GripTrackerTest::TestGrippable : public Grippable {
  public:
    GenericWidgetPtr widget;  ///< Widget to set in GripInfo.
    Point3f          target;  ///< Target point to set in GripInfo.
    Color            color;   ///< Color to set in GripInfo.

    virtual const SG::Node * GetGrippableNode() const override { return this; }

    virtual void UpdateGripInfo(GripInfo &info) override {
        info.widget       = widget;
        info.target_point = target;
        info.color        = color;
    }

  protected:
    TestGrippable() {}

  private:
    friend class Parser::Registry;
};

// ----------------------------------------------------------------------------
// GripTrackerTest functions.
// ----------------------------------------------------------------------------

GripTrackerTest::GripTrackerTest() {
    Parser::Registry::AddType<TestGrippable>("TestGrippable");
}

void GripTrackerTest::InitForActivation(GripTracker &gt) {
    // Set up a scene with GenericWidgets.
    InitTrackerScene(gt);

    // Use a TestGrippable..
    auto tg = CreateObject<TestGrippable>();
    if (gt.GetActuator() == Actuator::kLeftGrip) {
        tg->widget = GetLeftWidget();
        tg->target = Point3f(-1, 0, 0);
        tg->color  = Color(1, 0, 0);
    }
    else {
        tg->widget = GetRightWidget();
        tg->target = Point3f(1, 0, 0);
        tg->color  = Color(0, 0, 1);
    }

    gt.SetGrippable(tg, SG::NodePath(tg));
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
// GripTrackerTest Tests. Also tests PointerTracker.
// ----------------------------------------------------------------------------

TEST_F(GripTrackerTest, Defaults) {
    // Constructing with the wrong Actuator should assert.
    TEST_THROW(GripTracker(Actuator::kMouse), AssertException, "actuator");

    GripTracker lgt(Actuator::kLeftGrip);
    EXPECT_EQ(Actuator::kLeftGrip,            lgt.GetActuator());
    EXPECT_EQ(Event::Device::kLeftController, lgt.GetDevice());
    EXPECT_EQ(TK::kGripClickTimeout,          lgt.GetClickTimeout());

    GripTracker rgt(Actuator::kRightGrip);
    EXPECT_EQ(Actuator::kRightGrip,            rgt.GetActuator());
    EXPECT_EQ(Event::Device::kRightController, rgt.GetDevice());
    EXPECT_EQ(TK::kGripClickTimeout,           rgt.GetClickTimeout());
}

TEST_F(GripTrackerTest, IsActivation) {
    GripTracker lgt(Actuator::kLeftGrip);
    GripTracker rgt(Actuator::kRightGrip);
    WidgetPtr    lwidget, rwidget;

    InitForActivation(lgt);
    InitForActivation(rgt);

    EXPECT_TRUE(lgt.IsActivation(GetEvent(lgt, true), lwidget));
    EXPECT_TRUE(rgt.IsActivation(GetEvent(rgt, true), rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong device.
    Event levent = GetEvent(lgt, true);
    Event revent = GetEvent(rgt, true);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lgt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong button.
    levent = GetEvent(lgt, true);
    revent = GetEvent(rgt, true);
    levent.button = Event::Button::kPinch;
    revent.button = Event::Button::kPinch;
    EXPECT_FALSE(lgt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong hand.
    levent = GetEvent(rgt, true);
    revent = GetEvent(lgt, true);
    EXPECT_FALSE(lgt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Not a press.
    levent = GetEvent(lgt, false);
    revent = GetEvent(rgt, false);
    EXPECT_FALSE(lgt.IsActivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsActivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);
}

TEST_F(GripTrackerTest, IsDeactivation) {
    GripTracker lgt(Actuator::kLeftGrip);
    GripTracker rgt(Actuator::kRightGrip);
    WidgetPtr    lwidget, rwidget;

    InitForActivation(lgt);
    InitForActivation(rgt);

    EXPECT_TRUE(lgt.IsDeactivation(GetEvent(lgt, false), lwidget));
    EXPECT_TRUE(rgt.IsDeactivation(GetEvent(rgt, false), rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong device.
    Event levent = GetEvent(lgt, false);
    Event revent = GetEvent(rgt, false);
    levent.device = Event::Device::kHeadset;
    revent.device = Event::Device::kHeadset;
    EXPECT_FALSE(lgt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong button.
    levent = GetEvent(lgt, false);
    revent = GetEvent(rgt, false);
    levent.button = Event::Button::kPinch;
    revent.button = Event::Button::kPinch;
    EXPECT_FALSE(lgt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Wrong hand.
    levent = GetEvent(rgt, false);
    revent = GetEvent(lgt, false);
    EXPECT_FALSE(lgt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);

    // Not a press.
    levent = GetEvent(lgt, true);
    revent = GetEvent(rgt, true);
    EXPECT_FALSE(lgt.IsDeactivation(levent, lwidget));
    EXPECT_FALSE(rgt.IsDeactivation(revent, rwidget));
    EXPECT_NULL(lwidget);
    EXPECT_NULL(rwidget);
}

TEST_F(GripTrackerTest, ActivationWidget) {
    GripTracker lgt(Actuator::kLeftGrip);
    GripTracker rgt(Actuator::kRightGrip);
    InitForActivation(lgt);
    InitForActivation(rgt);

    // Set up events that will intersect the left and right GenericWidgets.
    Event levent = GetWidgetEvent(lgt);
    Event revent = GetWidgetEvent(rgt);

    auto lw = GetLeftWidget();
    auto rw = GetRightWidget();

    WidgetPtr lwidget, rwidget;
    EXPECT_TRUE(lgt.IsActivation(levent, lwidget));
    EXPECT_TRUE(rgt.IsActivation(revent, rwidget));
    EXPECT_EQ(lw, lwidget);
    EXPECT_EQ(rw, rwidget);
}

TEST_F(GripTrackerTest, Hover) {
    GripTracker lgt(Actuator::kLeftGrip);
    GripTracker rgt(Actuator::kRightGrip);
    InitForActivation(lgt);
    InitForActivation(rgt);

    // Set up events that will return the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lgt);
    Event revent = GetWidgetEvent(rgt);

    auto lw = GetLeftWidget();
    auto rw = GetRightWidget();

    EXPECT_FALSE(lw->IsHovering());
    EXPECT_FALSE(rw->IsHovering());
    lgt.UpdateHovering(levent);
    rgt.UpdateHovering(revent);
    EXPECT_TRUE(lw->IsHovering());
    EXPECT_TRUE(rw->IsHovering());

    lgt.StopHovering();
    rgt.StopHovering();
    EXPECT_FALSE(lw->IsHovering());
    EXPECT_FALSE(rw->IsHovering());

    // Cannot hover a disabled Widget.
    lw->SetInteractionEnabled(false);
    rw->SetInteractionEnabled(false);
    lgt.UpdateHovering(levent);
    rgt.UpdateHovering(revent);
    EXPECT_FALSE(lw->IsHovering());
    EXPECT_FALSE(rw->IsHovering());
}

#if XXXX
TEST_F(GripTrackerTest, ClickDrag) {
    GripTracker lgt(Actuator::kLeftGrip);
    GripTracker rgt(Actuator::kRightGrip);
    InitForActivation(lgt);
    InitForActivation(rgt);

    // Set up events that will intersect the left/right GenericWidgets.
    Event levent = GetWidgetEvent(lgt);
    Event revent = GetWidgetEvent(rgt);

    auto lw = GetLeftWidget();
    auto rw = GetRightWidget();

    // Activate to set the activation info.
    WidgetPtr lwidget, rwidget;
    EXPECT_TRUE(lgt.IsActivation(levent, lwidget));
    EXPECT_TRUE(rgt.IsActivation(revent, rwidget));

    // Update ClickInfos.
    ClickInfo lcinfo, rcinfo;
    lgt.FillClickInfo(lcinfo);
    rgt.FillClickInfo(rcinfo);
    EXPECT_EQ(Event::Device::kLeftController,  lcinfo.device);
    EXPECT_EQ(Event::Device::kRightController, rcinfo.device);
    EXPECT_EQ(lw.get(),                        lcinfo.widget);
    EXPECT_EQ(rw.get(),                        rcinfo.widget);

    // Update DragInfos.
    DragInfo ldinfo, rdinfo;
    lgt.FillActivationDragInfo(ldinfo);
    rgt.FillActivationDragInfo(rdinfo);
    EXPECT_EQ(Trigger::kPointer, ldinfo.trigger);
    EXPECT_EQ(Trigger::kPointer, rdinfo.trigger);
    EXPECT_EQ(lw,                ldinfo.hit.path.back());
    EXPECT_EQ(rw,                rdinfo.hit.path.back());

    // Not a drag if no position.
    levent.flags.Reset(Event::Flag::kPosition3D);
    revent.flags.Reset(Event::Flag::kPosition3D);
    EXPECT_FALSE(lgt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rgt.MovedEnoughForDrag(revent));
    levent.flags.Set(Event::Flag::kPosition3D);
    revent.flags.Set(Event::Flag::kPosition3D);

    // Not a drag if not far enough.
    levent.orientation = BuildRotation(0, 1, 0,  11);
    revent.orientation = BuildRotation(0, 1, 0, -11);
    EXPECT_FALSE(lgt.MovedEnoughForDrag(levent));
    EXPECT_FALSE(rgt.MovedEnoughForDrag(revent));

    // Drag if far enough.
    levent.orientation = BuildRotation(0, 1, 0,  15);
    revent.orientation = BuildRotation(0, 1, 0, -15);
    EXPECT_TRUE(lgt.MovedEnoughForDrag(levent));
    EXPECT_TRUE(rgt.MovedEnoughForDrag(revent));

    // Update the DragInfo. Note that the angle change is enough to miss the
    // GenericWidgets.
    lgt.FillEventDragInfo(levent, ldinfo);
    rgt.FillEventDragInfo(revent, rdinfo);
    EXPECT_EQ(Trigger::kPointer,  ldinfo.trigger);
    EXPECT_EQ(Trigger::kPointer,  rdinfo.trigger);
    EXPECT_FALSE(ldinfo.hit.IsValid());
    EXPECT_FALSE(rdinfo.hit.IsValid());

    lgt.Reset();
    rgt.Reset();
}
#endif
