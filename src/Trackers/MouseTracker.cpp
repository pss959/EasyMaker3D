#include "Trackers/MouseTracker.h"

#include "Debug/Shortcuts.h"
#include "Math/Frustum.h"
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
    widget.reset();
    return (event.flags.Has(Event::Flag::kButtonPress) &&
            event.device == Event::Device::kMouse &&
            event.button == Event::Button::kMouse1 &&
            GetCurrentWidget(event, true, widget));
}

bool MouseTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    widget.reset();
    return (event.flags.Has(Event::Flag::kButtonRelease) &&
            event.device == Event::Device::kMouse &&
            event.button == Event::Button::kMouse1 &&
            GetCurrentWidget(event, false, widget));
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
    if (debug_sphere_) {
        if (hit.IsValid()) {
            debug_sphere_->TranslateTo(hit.GetWorldPoint());
            debug_sphere_->SetEnabled(true);
            if (Util::app_type == Util::AppType::kMainApp)
                Debug::DisplayDebugText(hit.path.ToString());
        }
        else {
            if (Util::app_type == Util::AppType::kMainApp)
                Debug::DisplayDebugText("");
            debug_sphere_->SetEnabled(false);
        }
    }
    if (hit.IsValid())
        Debug::SetLimitPath(hit.path);
#endif  // LCOV_EXCL_STOP
}
