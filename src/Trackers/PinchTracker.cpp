#include "Trackers/PinchTracker.h"

#include "App/CoordConv.h"
#include "Base/Tuning.h"
#include "App/SceneContext.h"
#include "Items/Controller.h"
#include "SG/Search.h"

PinchTracker::PinchTracker(Actuator actuator) : PointerTracker(actuator) {
    ASSERT(actuator == Actuator::kLeftPinch ||
           actuator == Actuator::kRightPinch);
}

Event::Device PinchTracker::GetDevice() const {
    return GetActuator() == Actuator::kLeftPinch ?
        Event::Device::kLeftController : Event::Device::kRightController;
}

void PinchTracker::SetSceneContext(const SceneContextPtr &context) {
    PointerTracker::SetSceneContext(context);
    cdata.Init(*context, GetActuator() == Actuator::kLeftPinch ?
               Hand::kLeft : Hand::kRight);
}

bool PinchTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == GetDevice() && event.button == Event::Button::kPinch) {
        widget = ActivateWidget(event);
        UpdateControllers_(true);
        return true;
    }
    return false;
}

bool PinchTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == GetDevice() && event.button == Event::Button::kPinch) {
        widget = DeactivateWidget(event);
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
        const auto pt = cdata.ToControllerCoords(hit.GetWorldPoint());
        cdata.GetController().ShowPointerHover(true, pt);
    }
    else {
        cdata.GetController().ShowPointerHover(false, Point3f::Zero());
    }
}

void PinchTracker::UpdateControllers_(bool is_active) {
    cdata.GetController().SetTriggerMode(Trigger::kPointer, is_active);
    cdata.GetOtherController().ShowAll(! is_active);
}
