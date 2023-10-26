#include "Trackers/MouseTracker.h"

#include "Debug/Shortcuts.h"
#include "SG/Node.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"

static float s_click_timeout_ = TK::kMouseClickTimeout;

MouseTracker::MouseTracker(Actuator actuator) : PointerTracker(actuator) {
    ASSERT(actuator == Actuator::kMouse);
}

// LCOV_EXCL_START [debug only]
void MouseTracker::SetDebugSphere(const SG::NodePtr &ds) {
    debug_sphere_ = ds;
}

void MouseTracker::SetClickTimeout(float seconds) {
    s_click_timeout_ = seconds ? seconds : TK::kMouseClickTimeout;
}
// LCOV_EXCL_STOP

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
    widget.reset();
    return false;
}

bool MouseTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == Event::Device::kMouse &&
        event.button == Event::Button::kMouse1) {
        widget = GetCurrentWidget(event, false);
        return true;
    }
    widget.reset();
    return false;
}

float MouseTracker::GetClickTimeout() const {
    return s_click_timeout_;
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
#if ENABLE_DEBUG_FEATURES  // LCOV_EXCL_START [debug only]
    if (Util::app_type == Util::AppType::kMainApp && debug_sphere_) {
        auto &ds = *debug_sphere_;
        if (hit.IsValid()) {
            ds.TranslateTo(hit.GetWorldPoint());
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
#endif  // LCOV_EXCL_STOP
}
