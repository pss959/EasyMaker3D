#include "Trackers/PointerTracker.h"

#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/Intersector.h"
#include "Widgets/ClickableWidget.h"

void PointerTracker::UpdateHovering(const Event &event) {
    WidgetPtr widget;
    if (UpdateCurrentData_(event, widget) && widget != current_widget_) {
        UpdateWidgetHovering(current_widget_, widget);
        current_widget_ = widget;
    }
    if (widget)
        widget->UpdateHoverPoint(current_hit_.point);
}

void PointerTracker::StopHovering() {
    if (current_widget_)
        UpdateWidgetHovering(current_widget_, WidgetPtr());
}

void PointerTracker::FillActivationDragInfo(DragInfo &info) {
    info.trigger = Trigger::kPointer;
    info.ray     = activation_ray_;
    info.hit     = activation_hit_;
}

bool PointerTracker::MovedEnoughForDrag(const Event &event) {
    using ion::math::Normalized;
    using ion::math::AngleBetween;

    // Compute the current ray from the event.
    if (! GetRay(event, current_ray_))
        return false;

    // Check the ray direction change.
    const Vector3f d0 = Normalized(activation_ray_.direction);
    const Vector3f d1 = Normalized(current_ray_.direction);
    return AngleBetween(d0, d1) > GetMinRayAngleChange();
}

void PointerTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Ray ray;
    if (GetRay(event, ray)) {
        info.trigger = Trigger::kPointer;
        info.ray = ray;
        info.hit = SG::Intersector::IntersectScene(*GetScene(), ray);
    }
}

void PointerTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetDevice();
    info.hit    = current_hit_;
    info.widget =
        std::dynamic_pointer_cast<ClickableWidget>(current_widget_).get();
}

void PointerTracker::Reset() {
    activation_hit_ = current_hit_ = SG::Hit();
    current_widget_.reset();
}

WidgetPtr PointerTracker::GetCurrentWidget(const Event &event,
                                           bool is_activation) {
    UpdateCurrentData_(event, current_widget_);
    if (is_activation) {
        activation_ray_ = current_ray_;
        activation_hit_ = current_hit_;
    }
    return current_widget_;
}

bool PointerTracker::UpdateCurrentData_(const Event &event, WidgetPtr &widget) {
    widget.reset();

    Ray ray;
    if (! GetRay(event, ray))
        return false;

    SG::Hit hit = SG::Intersector::IntersectScene(*GetScene(), ray);

    current_ray_ = ray;
    current_hit_ = hit;

    if (! path_filter_ || path_filter_(hit.path)) {
        widget = hit.path.FindNodeUpwards<Widget>();
        if (widget && ! widget->IsInteractionEnabled())
            widget.reset();
    }

    // Let the derived class update based on the new Hit.
    ProcessCurrentHit(hit);
    return true;
}
