#include "Trackers/TouchTracker.h"

#include <ion/math/vectorutils.h>

#include "App/ClickInfo.h"
#include "App/DragInfo.h"
#include "App/SceneContext.h"
#include "Base/Event.h"
#include "Items/Controller.h"
#include "Math/Linear.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Widgets/ClickableWidget.h"

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

WidgetPtr TouchTracker::GetWidgetForEvent(const Event &event) {
    WidgetPtr widget;
    // XXXX
    return widget;
}

void TouchTracker::SetActive(bool is_active) {
    // XXXX Vibrate here?
}

bool TouchTracker::MovedEnoughForDrag(const Event &event) {
#if XXXX
    /// Minimum world-space distance for a controller to move to be considered
    // a potential touch drag operation.
    const float  kMinDragDistance = .04f;

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(touched_widget_).get();
    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;

    // Check for position change.
    const Point3f   &p0 = activation_data_.position;
    const Point3f   &p1 =    current_data_.position;
    return ion::math::Distance(p0, p1) > scale * kMinDragDistance;
#endif
    return false;
}

void TouchTracker::FillActivationDragInfo(DragInfo &info) {
    // XXXX
}

void TouchTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    // XXXX
}

void TouchTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetActuator() == Actuator::kLeftTouch ?
        Event::Device::kLeftController : Event::Device::kRightController;
    info.widget = Util::CastToDerived<ClickableWidget>(touched_widget_).get();
}

void TouchTracker::Reset() {
    touched_widget_.reset();
}

bool TouchTracker::IsTouchEvent_(const Event &event) const {
    return event.flags.Has(Event::Flag::kTouch) &&
        ((GetActuator() == Actuator::kLeftTouch &&
          event.device == Event::Device::kLeftController) ||
         (GetActuator() == Actuator::kRightTouch &&
          event.device == Event::Device::kRightController));
}
