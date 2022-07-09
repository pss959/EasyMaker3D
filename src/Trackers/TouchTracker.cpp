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

void TouchTracker::SetActive(bool is_active) {
    controller_->Vibrate(.05f);
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
    info.is_grip  = false;
    info.ray      = activation_ray_;
    info.hit      = activation_hit_;
}

void TouchTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Point3f pos;
    if (GetTouchPos_(event, pos)) {
        info.is_grip = false;
        info.ray = Ray(pos, -Vector3f::AxisZ());
        info.hit = SG::Intersector::IntersectScene(*GetContext().scene,
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
