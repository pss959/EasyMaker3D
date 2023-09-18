#include "Trackers/TouchTracker.h"

#include <ion/math/vectorutils.h>

#include "Base/Event.h"
#include "Items/Controller.h"
#include "Math/Linear.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/Intersector.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/ClickableWidget.h"
#include "Widgets/Touchable.h"

TouchTracker::TouchTracker(Actuator actuator) : Tracker(actuator) {
    ASSERT(actuator == Actuator::kLeftTouch ||
           actuator == Actuator::kRightTouch);
}

Event::Device TouchTracker::GetDevice() const {
    return IsLeft() ?
        Event::Device::kLeftController : Event::Device::kRightController;
}

bool TouchTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    // A Touchable has to be present for this to activate.
    auto controller = GetController();
    ASSERT(controller);
    const float radius = GetController()->GetTouchRadius();
    Point3f pos;
    if (touchable_ && GetTouchPos_(event, pos)) {
        widget = touchable_->GetTouchedWidget(pos, radius);
        if (widget && widget->IsInteractionEnabled()) {
            activation_pos_ = pos;
            current_widget_ = widget;
            GetController()->ShowTouch(true);
            return true;
        }
    }
    widget.reset();
    return false;
}

bool TouchTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    ASSERT(touchable_);
    ASSERT(current_widget_);

    // Deactivation requires pulling the touch affordance back (+Z)
    // sufficiently.
    Point3f pos;
    if (GetTouchPos_(event, pos) &&
        pos[2] >= activation_pos_[2] + TK::kMinTouchZMotion) {
        const float radius = GetController()->GetTouchRadius();
        widget = touchable_->GetTouchedWidget(pos, radius);
        // Deactivation requires pulling the touch affordance back (+Z)
        // sufficiently to stop intersecting the current widget. This is a
        // deactivation if this is true and now touching nothing or a different
        // Widget.
        if (widget != current_widget_) {
            // If no longer touching any Widget, this is a valid touch.  If now
            // touching a different Widget, this is not a valid touch; just
            // return the new Widget.
            if (! widget) {
                GetController()->ShowTouch(false);
                widget = current_widget_;
            }
            return true;
        }
    }
    widget.reset();
    return false;
}

void TouchTracker::FillActivationDragInfo(DragInfo &info) {
    info.trigger        = Trigger::kTouch;
    info.touch_position = activation_pos_;
}

bool TouchTracker::MovedEnoughForDrag(const Event &event) {
    if (! event.flags.Has(Event::Flag::kTouch) || event.device != GetDevice())
        return false;

    // Check for sufficient X or Y position change.
    const float motion_scale = GetMotionScale(current_widget_);
    const Point2f p0 = ToPoint2f(activation_pos_);
    const Point2f p1 = ToPoint2f(event.touch_position3D);
    const float distance = ion::math::Distance(p0, p1);
    return motion_scale * distance > TK::kMinTouchControllerDistance;
}

void TouchTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Point3f pos;
    if (GetTouchPos_(event, pos)) {
        info.trigger        = Trigger::kTouch;
        info.touch_position = pos;
    }
}

void TouchTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetDevice();
    info.widget =
        std::dynamic_pointer_cast<ClickableWidget>(current_widget_).get();
}

void TouchTracker::Reset() {
    current_widget_.reset();
}

bool TouchTracker::GetTouchPos_(const Event &event, Point3f &pos) const {
    if (event.flags.Has(Event::Flag::kTouch) &&
        event.device == GetDevice()) {
        pos = event.touch_position3D;
        return true;
    }
    return false;
}
