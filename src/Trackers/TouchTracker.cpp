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

#if XXXX
WidgetPtr TouchTracker::GetWidgetForEvent(const Event &event) {
    WidgetPtr widget;

    // Get the touch position if the event has one for this actuator.
    Point3f pos;
    if (! GetTouchPos_(event, pos))
        return widget;

    Ray ray(pos, -Vector3f::AxisZ());
    SG::Hit hit = SG::Intersector::IntersectScene(*GetContext().scene, ray);
    if (hit.IsValid()) {
        // The touch affordance has to be within the kTouchRadius in front or a
        // multiple of the radius behind the widget (allowing for piercing to
        // work).
        const float dist = hit.distance;
        if ((dist >= 0 && dist <= Defaults::kTouchRadius) ||
            (dist <  0 && dist >= -4 * Defaults::kTouchRadius))
            widget = hit.path.FindNodeUpwards<Widget>();
        activation_ray_ = ray;
        activation_hit_ = hit;
    }

    touched_widget_ = widget;
    return widget;
}
#endif

bool TouchTracker::IsActivation(const Event &event, WidgetPtr &widget) {
#if XXXX
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == GetDevice_() && event.button == Event::Button::kTouch) {
        current_widget_ = UpdateCurrentData_(event);
        if (current_widget_) {
            if (current_widget_->IsHovering())
                current_widget_->SetHovering(false);
            current_widget_->SetActive(true);
        }
        activation_data_ = current_data_;
        controller_->Vibrate(.05f);
        widget = current_widget_;
        return true;
    }
#endif
    return false;
}

bool TouchTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
#if XXXX
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == GetDevice_() && event.button == Event::Button::kTouch) {
        if (current_widget_)
            current_widget_->SetActive(false);
        UpdateControllers_(false);
        controller_->Vibrate(.05f);
        widget = UpdateCurrentData_(event);
        return true;
    }
#endif
    return false;
}

bool TouchTracker::MovedEnoughForDrag(const Event &event) {
    Point3f pos;
    if (! GetTouchPos_(event, pos))
        return false;

    /// Minimum world-space distance for a controller to move to be considered
    // a potential touch drag operation.
    const float  kMinDragDistance = .04f;

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(touched_widget_).get();
    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;

    // Check for sufficient position change.
    const Point3f &p0 = activation_ray_.origin;
    const Point3f &p1 = pos;
    return ion::math::Distance(p0, p1) > scale * kMinDragDistance;
}

void TouchTracker::FillActivationDragInfo(DragInfo &info) {
    info.trigger = Trigger::kTouch;
    info.ray     = activation_ray_;
    info.hit     = activation_hit_;
}

void TouchTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Point3f pos;
    if (GetTouchPos_(event, pos)) {
        info.trigger = Trigger::kTouch;
        info.ray     = Ray(pos, -Vector3f::AxisZ());
        info.hit     = SG::Intersector::IntersectScene(*GetContext().scene,
                                                       info.ray);
    }
}

void TouchTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetActuator() == Actuator::kLeftTouch ?
        Event::Device::kLeftController : Event::Device::kRightController;
    info.widget = Util::CastToDerived<ClickableWidget>(touched_widget_).get();
}

void TouchTracker::Reset() {
    touched_widget_.reset();
}

bool TouchTracker::GetTouchPos_(const Event &event, Point3f &pos) const {
    if (event.flags.Has(Event::Flag::kTouch) &&
        ((GetActuator() == Actuator::kLeftTouch &&
          event.device == Event::Device::kLeftController) ||
         (GetActuator() == Actuator::kRightTouch &&
          event.device == Event::Device::kRightController))) {
        pos = event.touch_position3D;
        return true;
    }
    return false;
}
