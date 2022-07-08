#include "Trackers/PointerTracker.h"

#include "App/ClickInfo.h"
#include "App/DragInfo.h"
#include "App/SceneContext.h"
#include "SG/Intersector.h"
#include "Util/General.h"
#include "Widgets/ClickableWidget.h"

WidgetPtr PointerTracker::GetWidgetForEvent(const Event &event) {
    WidgetPtr widget;

    Ray ray;
    if (! GetRay(event, ray))
        return widget;

    SG::Hit hit = SG::Intersector::IntersectScene(*GetContext().scene, ray);
    if (! path_filter_ || path_filter_(hit.path))
        widget = hit.path.FindNodeUpwards<Widget>();

    current_ray_ = ray;
    current_hit_ = hit;

    if (widget != hovered_widget_) {
        UpdateHover(hovered_widget_, widget);
        hovered_widget_ = widget;
        ProcessCurrentHit(hit);
    }

    return widget;
}

void PointerTracker::SetActive(bool is_active) {
    if (is_active) {
        activation_ray_ = current_ray_;
        activation_hit_ = current_hit_;
    }
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
        Util::CastToDerived<ClickableWidget>(hovered_widget_).get();

    // Check the ray direction change.
    const Vector3f d0 = ion::math::Normalized(activation_ray_.direction);
    const Vector3f d1 = ion::math::Normalized(current_ray_.direction);

    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;
    return ion::math::AngleBetween(d0, d1) > scale * kMinRayAngle;
}

void PointerTracker::FillActivationDragInfo(DragInfo &info) {
    info.is_grip = false;
    info.ray     = activation_ray_;
    info.hit     = activation_hit_;
}

void PointerTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Ray ray;
    if (GetRay(event, ray)) {
        info.is_grip = false;
        info.ray = ray;
        info.hit = SG::Intersector::IntersectScene(*GetContext().scene, ray);
    }
}

void PointerTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetDevice();
    info.hit    = current_hit_;
    info.widget = Util::CastToDerived<ClickableWidget>(hovered_widget_).get();
}

void PointerTracker::Reset() {
    activation_hit_ = current_hit_ = SG::Hit();
    hovered_widget_.reset();
}
