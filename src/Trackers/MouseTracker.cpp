#include "Trackers/MouseTracker.h"

#include "Debug/Shortcuts.h"
#include "SG/Node.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

MouseTracker::MouseTracker(Actuator actuator) : PointerTracker(actuator) {
    ASSERT(actuator == Actuator::kMouse);
}

Event::Device MouseTracker::GetDevice() const {
    return Event::Device::kMouse;
}

bool MouseTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == Event::Device::kMouse &&
        event.button == Event::Button::kMouse1) {
        widget = GetCurrentWidget(event, true);
        return true;
    }
    return false;
}

bool MouseTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == Event::Device::kMouse &&
        event.button == Event::Button::kMouse1) {
        widget = GetCurrentWidget(event, false);
        return true;
    }
    return false;
}

float MouseTracker::GetClickTimeout() const {
    return TK::kMouseClickTimeout;
}

bool MouseTracker::GetRay(const Event &event, Ray &ray) {
    if (event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {
        ASSERT(frustum_);
        ray = frustum_->BuildRay(event.position2D);
        return true;
    }
    return false;
}

Anglef MouseTracker::GetMinRayAngleChange() const {
    return TK::kMinMouseRayAngleChange;
}

void MouseTracker::ProcessCurrentHit(const SG::Hit &hit) {
#if ENABLE_DEBUG_FEATURES
    if (debug_sphere_) {
        auto &ds = *debug_sphere_;
        if (hit.IsValid()) {
            ds.SetTranslation(hit.GetWorldPoint());
            ds.SetEnabled(true);
            Debug::DisplayDebugText(hit.path.ToString());
        }
        else {
            Debug::DisplayDebugText("");
            ds.SetEnabled(false);
        }
    }
    if (hit.IsValid())
        Debug::SetLimitPath(hit.path);
#endif
}
