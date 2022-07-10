#include "Trackers/PointerTracker.h"

#include "App/ClickInfo.h"
#include "App/DragInfo.h"
#include "App/SceneContext.h"
#include "SG/Intersector.h"
#include "Util/General.h"
#include "Widgets/ClickableWidget.h"

void PointerTracker::UpdateHovering(const Event &event) {
    WidgetPtr prev_widget = current_widget_;

    current_widget_ = UpdateCurrentData_(event);

    if (current_widget_ != prev_widget)
        UpdateWidgetHovering(prev_widget, current_widget_);
}

void PointerTracker::StopHovering() {
    if (current_widget_)
        UpdateWidgetHovering(current_widget_, WidgetPtr());
}

bool PointerTracker::MovedEnoughForDrag(const Event &event) {
    // Compute the current ray from the event.
    if (! GetRay(event, current_ray_))
        return false;

    /// Minimum angle between two ray directions to be considered enough for a
    // drag.
    const Anglef kMinRayAngle = Anglef::FromDegrees(2);

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(current_widget_).get();

    // Check the ray direction change.
    const Vector3f d0 = ion::math::Normalized(activation_ray_.direction);
    const Vector3f d1 = ion::math::Normalized(current_ray_.direction);

    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;
    return ion::math::AngleBetween(d0, d1) > scale * kMinRayAngle;
}

void PointerTracker::FillActivationDragInfo(DragInfo &info) {
    info.trigger = Trigger::kPointer;
    info.ray     = activation_ray_;
    info.hit     = activation_hit_;
}

void PointerTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Ray ray;
    if (GetRay(event, ray)) {
        info.trigger = Trigger::kPointer;
        info.ray = ray;
        info.hit = SG::Intersector::IntersectScene(*GetContext().scene, ray);
    }
}

void PointerTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetDevice();
    info.hit    = current_hit_;
    info.widget = Util::CastToDerived<ClickableWidget>(current_widget_).get();
}

void PointerTracker::Reset() {
    activation_hit_ = current_hit_ = SG::Hit();
    current_widget_.reset();
}

WidgetPtr PointerTracker::ActivateWidget(const Event &event) {
    current_widget_ = UpdateCurrentData_(event);
    if (current_widget_) {
        if (current_widget_->IsHovering())
            current_widget_->SetHovering(false);
        current_widget_->SetActive(true);
    }
    activation_ray_ = current_ray_;
    activation_hit_ = current_hit_;
    return current_widget_;
}

WidgetPtr PointerTracker::DeactivateWidget(const Event &event) {
    if (current_widget_)
        current_widget_->SetActive(false);
    return UpdateCurrentData_(event);
}

WidgetPtr PointerTracker::UpdateCurrentData_(const Event &event) {
    WidgetPtr widget;

    Ray ray;
    if (GetRay(event, ray)) {
        SG::Hit hit = SG::Intersector::IntersectScene(*GetContext().scene, ray);

        current_ray_ = ray;
        current_hit_ = hit;

        if (! path_filter_ || path_filter_(hit.path))
            widget = hit.path.FindNodeUpwards<Widget>();

        // Let the derived class update based on the new Hit.
        ProcessCurrentHit(hit);
    }
    return widget;
}
