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

    /// Returns an Event for a mouse press, release, or motion. \p action is
    /// "press", "release", or empty for neither. \p widget is "L" to position
    /// over the left GenericWidget), "R" for the right GenericWidget, or empty
    /// for neither.
    static Event GetEvent(const Str &action, const Str &widget);
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

Event MouseTrackerTest::GetEvent(const Str &action, const Str &widget) {
    Event event;
    event.device = Event::Device::kMouse;
    if (action == "press" || action == "release") {
        event.button = Event::Button::kMouse1;
        event.flags.Set(action == "press" ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
    }
    event.flags.Set(Event::Flag::kPosition2D);
    if (widget == "L")
        event.position2D.Set(.3f, .5f);
    else if (widget == "R")
        event.position2D.Set(.6f, .5f);
    else
        event.position2D.Set(0, 0);
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
    // Activation with no Widget.
    Event event = GetEvent("press", "");
    EXPECT_TRUE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);

    // Activation with left GenericWidget.
    event = GetEvent("press", "L");
    EXPECT_TRUE(mt.IsActivation(event, tw));
    EXPECT_EQ(lgw, tw);

    // Wrong device.
    event.device = Event::Device::kHeadset;
    EXPECT_FALSE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);

    // Wrong button.
    event = GetEvent("press", "");
    event.button = Event::Button::kMouse2;
    EXPECT_FALSE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);

    // Not a press.
    event = GetEvent("release", "");
    EXPECT_FALSE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);
    event = GetEvent("", "");
    EXPECT_FALSE(mt.IsActivation(event, tw));
    EXPECT_NULL(tw);
}

TEST_F(MouseTrackerTest, IsDeactivation) {
    // Deactivation with no Widget.
    Event event = GetEvent("release", "");
    EXPECT_TRUE(mt.IsDeactivation(event, tw));

    // Deactivation with right GenericWidget.
    event = GetEvent("release", "R");
    EXPECT_TRUE(mt.IsDeactivation(event, tw));
    EXPECT_EQ(rgw, tw);

    // Wrong device.
    event = GetEvent("release", "");
    event.device = Event::Device::kHeadset;
    EXPECT_FALSE(mt.IsDeactivation(event, tw));
    EXPECT_NULL(tw);

    // Wrong button.
    event = GetEvent("release", "");
    event.button = Event::Button::kMouse2;
    EXPECT_FALSE(mt.IsDeactivation(event, tw));
    EXPECT_NULL(tw);

    // Not a release.
    event = GetEvent("press", "");
    EXPECT_FALSE(mt.IsDeactivation(event, tw));
    EXPECT_NULL(tw);
    event = GetEvent("", "");
    EXPECT_FALSE(mt.IsDeactivation(event, tw));
    EXPECT_NULL(tw);
}

TEST_F(MouseTrackerTest, Hover) {
    // Set up an event that will intersect the left GenericWidget.
    Event event = GetEvent("", "L");
    EXPECT_FALSE(lgw->IsHovering());
    mt.UpdateHovering(event);
    EXPECT_TRUE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());

    // Not over a Widget - stop hovering.
    event = GetEvent("", "");
    mt.UpdateHovering(event);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());

    // Install a path filter that always returns false.
    auto filter = [](const SG::NodePath &){ return false; };
    mt.SetPathFilter(filter);
    event = GetEvent("", "R");
    mt.UpdateHovering(event);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());

    // Remove the filter.
    mt.SetPathFilter(nullptr);
    mt.UpdateHovering(event);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_TRUE(rgw->IsHovering());

    mt.StopHovering();
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());

    // Cannot hover a disabled Widget.
    rgw->SetInteractionEnabled(false);
    mt.UpdateHovering(event);
    EXPECT_FALSE(lgw->IsHovering());
    EXPECT_FALSE(rgw->IsHovering());
}

TEST_F(MouseTrackerTest, ClickDrag) {
    // Activate the right GenericWidget to set the activation info.
    Event event = GetEvent("press", "R");
    EXPECT_TRUE(mt.IsActivation(event, tw));
    EXPECT_EQ(rgw, tw);

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
    event = GetEvent("", "R");
    event.flags.Reset(Event::Flag::kPosition2D);
    EXPECT_FALSE(mt.MovedEnoughForDrag(event));

    // Not a drag if not far enough.
    event = GetEvent("", "R");
    event.position2D[0] += .0001f;
    EXPECT_FALSE(mt.MovedEnoughForDrag(event));

    // Drag if far enough.
    event.position2D[0] = .7f;
    EXPECT_TRUE(mt.MovedEnoughForDrag(event));

    // Update the DragInfo.
    mt.FillEventDragInfo(event, dinfo);
    EXPECT_ENUM_EQ(Trigger::kPointer, dinfo.trigger);
    EXPECT_EQ(rgw,                    dinfo.hit.path.back());

    mt.Reset();
}
