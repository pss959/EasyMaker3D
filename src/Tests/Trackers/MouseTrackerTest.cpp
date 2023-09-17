#include "Base/Event.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/WindowCamera.h"
#include "Tests/Testing.h"
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
    /// Sets up the scene and frustum in a MouseTracker.
    void InitForActivation(MouseTracker &mt);

    /// Returns an Event for a mouse press or release.
    static Event GetEvent(bool is_press);
};

void MouseTrackerTest::InitForActivation(MouseTracker &mt) {
    // Set up a scene with GenericWidgets.
    InitTrackerScene(mt);

    // Set up the frustum in the MouseTracker.
    FrustumPtr frustum(new Frustum);
    GetWindowCamera()->BuildFrustum(Vector2i(100, 100), *frustum);
    mt.SetFrustum(frustum);
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
    TEST_THROW(MouseTracker(Actuator::kLeftPinch), AssertException,
               "Actuator::kMouse");

    MouseTracker mt(Actuator::kMouse);
    EXPECT_ENUM_EQ(Actuator::kMouse,      mt.GetActuator());
    EXPECT_ENUM_EQ(Event::Device::kMouse, mt.GetDevice());
    EXPECT_EQ(TK::kMouseClickTimeout,     mt.GetClickTimeout());
}

TEST_F(MouseTrackerTest, IsActivation) {
    MouseTracker mt(Actuator::kMouse);
    WidgetPtr    widget;

    EXPECT_TRUE(mt.IsActivation(GetEvent(true), widget));
    EXPECT_NULL(widget);

    // Wrong device.
    Event event = GetEvent(true);
    event.device = Event::Device::kHeadset;
    EXPECT_FALSE(mt.IsActivation(event, widget));
    EXPECT_NULL(widget);

    // Wrong button.
    event = GetEvent(true);
    event.button = Event::Button::kMouse2;
    EXPECT_FALSE(mt.IsActivation(event, widget));
    EXPECT_NULL(widget);

    // Not a press.
    event = GetEvent(false);
    EXPECT_FALSE(mt.IsActivation(event, widget));
    EXPECT_NULL(widget);
}

TEST_F(MouseTrackerTest, IsDeactivation) {
    MouseTracker mt(Actuator::kMouse);
    WidgetPtr    widget;

    EXPECT_TRUE(mt.IsDeactivation(GetEvent(false), widget));
    EXPECT_NULL(widget);

    // Wrong device.
    Event event = GetEvent(false);
    event.device = Event::Device::kHeadset;
    EXPECT_FALSE(mt.IsDeactivation(event, widget));
    EXPECT_NULL(widget);

    // Wrong button.
    event = GetEvent(false);
    event.button = Event::Button::kMouse2;
    EXPECT_FALSE(mt.IsDeactivation(event, widget));
    EXPECT_NULL(widget);

    // Not a press.
    event = GetEvent(true);
    EXPECT_FALSE(mt.IsDeactivation(event, widget));
    EXPECT_NULL(widget);
}

TEST_F(MouseTrackerTest, ActivationWidget) {
    MouseTracker mt(Actuator::kMouse);
    InitForActivation(mt);

    // Set up an event that will intersect the left GenericWidget.
    Event event = GetEvent(true);
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(.3f, .5f);

    auto lw = GetLeftWidget();

    WidgetPtr widget;
    EXPECT_TRUE(mt.IsActivation(event, widget));
    EXPECT_EQ(lw, widget);
}

TEST_F(MouseTrackerTest, Hover) {
    MouseTracker mt(Actuator::kMouse);
    InitForActivation(mt);

    // Set up an event that will intersect the left GenericWidget.
    Event event = GetEvent(true);
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(.5f, .5f);

    auto lw = GetLeftWidget();
    EXPECT_FALSE(lw->IsHovering());
    mt.UpdateHovering(event);
    EXPECT_TRUE(lw->IsHovering());

    // Install a path filter that ignores the GenericWidgets.
    auto filter = [](const SG::NodePath &){ return false; };
    mt.SetPathFilter(filter);
    mt.UpdateHovering(event);
    EXPECT_FALSE(lw->IsHovering());
    mt.SetPathFilter(nullptr);
    mt.UpdateHovering(event);
    EXPECT_TRUE(lw->IsHovering());

    mt.StopHovering();
    EXPECT_FALSE(lw->IsHovering());

    // Cannot hover a disabled Widget.
    lw->SetInteractionEnabled(false);
    mt.UpdateHovering(event);
    EXPECT_FALSE(lw->IsHovering());
}

TEST_F(MouseTrackerTest, ClickDrag) {
    MouseTracker mt(Actuator::kMouse);
    InitForActivation(mt);

    auto rw = GetRightWidget();

    // Activate to set the activation info.
    Event event = GetEvent(true);
    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D.Set(.6f, .5f);
    WidgetPtr widget;
    EXPECT_TRUE(mt.IsActivation(event, widget));

    // Update a ClickInfo.
    ClickInfo cinfo;
    mt.FillClickInfo(cinfo);
    EXPECT_ENUM_EQ(Event::Device::kMouse, cinfo.device);
    EXPECT_EQ(rw.get(),                   cinfo.widget);

    // Update a DragInfo.
    DragInfo dinfo;
    mt.FillActivationDragInfo(dinfo);
    EXPECT_ENUM_EQ(Trigger::kPointer, dinfo.trigger);
    EXPECT_EQ(rw,                     dinfo.hit.path.back());

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
    EXPECT_EQ(rw,                     dinfo.hit.path.back());

    mt.Reset();
}
