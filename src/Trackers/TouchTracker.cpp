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
#include "Widgets/Touchable.h"

TouchTracker::TouchTracker(Actuator actuator) : Tracker(actuator) {
    ASSERT(actuator == Actuator::kLeftTouch ||
           actuator == Actuator::kRightTouch);
}

void TouchTracker::SetSceneContext(const SceneContextPtr &context) {
    Tracker::SetSceneContext(context);
    cdata.Init(*context, GetActuator() == Actuator::kLeftTouch ?
               Hand::kLeft : Hand::kRight);
}

bool TouchTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    // An Touchable has to be present for this to activate.
    const float radius = cdata.GetController().GetTouchRadius();
    Point3f pos;
    if (touchable_ && GetTouchPos_(event, pos)) {
        widget = touchable_->GetTouchedWidget(pos, radius);
        if (widget) {
            activation_pos_ = pos;
            current_widget_ = widget;
            cdata.GetController().ShowTouch(true);
            return true;
        }
    }
    return false;
}

bool TouchTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    ASSERT(touchable_);
    ASSERT(current_widget_);
    const float radius = cdata.GetController().GetTouchRadius();
    Point3f pos;
    if (GetTouchPos_(event, pos)) {
        widget = touchable_->GetTouchedWidget(pos, radius);
        // This is a deactivation if now touching nothing or a different
        // Widget.
        if (widget != current_widget_) {
            // If not touching a Widget, this is a valid touch.  If touching a
            // different Widget, this is not a valid touch; just return the new
            // Widget.
            if (! widget) {
                cdata.GetController().ShowTouch(false);
                widget = current_widget_;
            }
            return true;
        }
    }
    return false;
}

bool TouchTracker::MovedEnoughForDrag(const Event &event) {
    if (! event.flags.Has(Event::Flag::kTouch) ||
        event.device != cdata.GetDevice())
        return false;

    // Minimum world-space distance for a controller to move in X or Y to be
    // considered a potential touch drag operation.
    const float kMinDragDistance = .01f;

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(current_widget_).get();

    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;

    // Check for sufficient X or Y position change.
    const Point2f p0 = WithoutDimension(activation_pos_,        2);
    const Point2f p1 = WithoutDimension(event.touch_position3D, 2);
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
    info.device = cdata.GetDevice();
    info.widget = Util::CastToDerived<ClickableWidget>(current_widget_).get();
}

void TouchTracker::Reset() {
    current_widget_.reset();
}

bool TouchTracker::GetTouchPos_(const Event &event, Point3f &pos) const {
    if (event.flags.Has(Event::Flag::kTouch) &&
        event.device == cdata.GetDevice()) {
        pos = event.touch_position3D;
        return true;
    }
    return false;
}
