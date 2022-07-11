#include "Trackers/TouchTracker.h"

#include <ion/math/vectorutils.h>

#include "App/ClickInfo.h"
#include "App/DragInfo.h"
#include "App/SceneContext.h"
#include "Base/Defaults.h"
#include "Base/Event.h"
#include "Items/Controller.h"
#include "Math/Linear.h"
#include "SG/Intersector.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/ITouchable.h"

TouchTracker::TouchTracker(Actuator actuator) : Tracker(actuator) {
    ASSERT(actuator == Actuator::kLeftTouch ||
           actuator == Actuator::kRightTouch);
}

void TouchTracker::SetSceneContext(const SceneContextPtr &context) {
    Tracker::SetSceneContext(context);

    controller_ = GetActuator() == Actuator::kLeftTouch ?
        context->left_controller : context->right_controller;
    controller_path_ = SG::FindNodePathInScene(*context->scene, *controller_);
}

bool TouchTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    // An ITouchable has to be present for this to activate.
    Point3f pos;
    if (touchable_ && GetTouchPos_(event, pos)) {
        widget = touchable_->GetTouchedWidget(pos, Defaults::kTouchRadius);
        current_widget_ = widget;
        return widget.get();
    }
    return false;
}

bool TouchTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    ASSERT(touchable_);
    ASSERT(current_widget_);
    Point3f pos;
    if (GetTouchPos_(event, pos)) {
        // Deactivate if no longer touching the activated Widget.
        WidgetPtr prev_widget = current_widget_;
        current_widget_ =
            touchable_->GetTouchedWidget(pos, Defaults::kTouchRadius);
        return current_widget_ != prev_widget;
    }
    return false;
}

bool TouchTracker::MovedEnoughForDrag(const Event &event) {
    if (! event.flags.Has(Event::Flag::kTouch) || event.device != GetDevice_())
        return false;

    // Minimum world-space distance for a controller to move to be considered
    // a potential touch drag operation.
    const float kMinDragDistance = .04f;

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(current_widget_).get();

    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;

    // Check for sufficient position change.
    const Point3f &p0 = activation_pos_;
    const Point3f &p1 = event.touch_position3D;
    return ion::math::Distance(p0, p1) > scale * kMinDragDistance;
}

void TouchTracker::FillActivationDragInfo(DragInfo &info) {
    info.trigger        = Trigger::kTouch;
    info.touch_position = activation_pos_;
}

void TouchTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Point3f pos;
    if (GetTouchPos_(event, pos)) {
        info.trigger        = Trigger::kTouch;
        info.touch_position = pos;
    }
}

void TouchTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetDevice_();
    info.widget = Util::CastToDerived<ClickableWidget>(current_widget_).get();
}

void TouchTracker::Reset() {
    current_widget_.reset();
}

Event::Device TouchTracker::GetDevice_() const {
    return GetActuator() == Actuator::kLeftGrip ?
        Event::Device::kLeftController : Event::Device::kRightController;
}

bool TouchTracker::GetTouchPos_(const Event &event, Point3f &pos) const {
    if (event.flags.Has(Event::Flag::kTouch) && event.device == GetDevice_()) {
        pos = event.touch_position3D;
        return true;
    }
    return false;
}
