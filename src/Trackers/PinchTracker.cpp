//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Trackers/PinchTracker.h"

#include "Items/Controller.h"
#include "SG/CoordConv.h"
#include "SG/Search.h"
#include "Util/Tuning.h"

PinchTracker::PinchTracker(Actuator actuator) : PointerTracker(actuator) {
    ASSERT(actuator == Actuator::kLeftPinch ||
           actuator == Actuator::kRightPinch);
}

Event::Device PinchTracker::GetDevice() const {
    return IsLeft() ?
        Event::Device::kLeftController : Event::Device::kRightController;
}

bool PinchTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    widget.reset();
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == GetDevice() && event.button == Event::Button::kPinch &&
        GetCurrentWidget(event, true, widget)) {
        UpdateControllers_(true);
        return true;
    }
    return false;
}

bool PinchTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    widget.reset();
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == GetDevice() && event.button == Event::Button::kPinch &&
        GetCurrentWidget(event, false, widget)) {
        UpdateControllers_(false);
        return true;
    }
    return false;
}

float PinchTracker::GetClickTimeout() const {
    return TK::kPinchClickTimeout;
}

bool PinchTracker::GetRay(const Event &event, Ray &ray) {
    if (event.device == GetDevice() &&
        event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        ray = Ray(event.position3D, event.orientation * -Vector3f::AxisZ());
        return true;
    }
    return false;
}

Anglef PinchTracker::GetMinRayAngleChange() const {
    return TK::kMinPinchRayAngleChange;
}

void PinchTracker::ProcessCurrentHit(const SG::Hit &hit) {
    if (hit.IsValid()) {
        const auto pt = ToControllerCoords(hit.GetWorldPoint());
        GetController()->ShowPointerHover(true, pt);
    }
    else {
        GetController()->ShowPointerHover(false, Point3f::Zero());
    }
}

void PinchTracker::UpdateControllers_(bool is_active) {
    GetController()->SetTriggerMode(Trigger::kPointer, is_active);
    GetOtherController()->ShowAll(! is_active);
}
