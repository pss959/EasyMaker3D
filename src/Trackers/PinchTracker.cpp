#include "Trackers/PinchTracker.h"

#include "App/CoordConv.h"
#include "App/SceneContext.h"
#include "Items/Controller.h"
#include "SG/Search.h"

PinchTracker::PinchTracker(Actuator actuator) : PointerTracker(actuator) {
    ASSERT(actuator == Actuator::kLeftPinch ||
           actuator == Actuator::kRightPinch);
}

void PinchTracker::SetSceneContext(const SceneContextPtr &context) {
    PointerTracker::SetSceneContext(context);

    controller_ = GetActuator() == Actuator::kLeftPinch ?
        context->left_controller : context->right_controller;
    controller_path_ = SG::FindNodePathInScene(*context->scene, *controller_);
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

Event::Device PinchTracker::GetDevice() const {
    return GetActuator() == Actuator::kLeftPinch ?
        Event::Device::kLeftController : Event::Device::kRightController;
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

void PinchTracker::ProcessCurrentHit(const SG::Hit &hit) {
    ASSERT(controller_);
    if (hit.IsValid()) {
        const auto pt =
            CoordConv(controller_path_).RootToObject(hit.GetWorldPoint());
        controller_->ShowPointerHover(true, pt);
    }
    else {
        controller_->ShowPointerHover(false, Point3f::Zero());
    }
}

void PinchTracker::UpdateControllers_(bool is_active) {
    const auto &context = GetContext();
    controller_->ShowActive(is_active, false);
    const auto &other_controller = controller_ == context.left_controller ?
        context.right_controller : context.left_controller;
    other_controller->ShowPointer(! is_active);
    other_controller->ShowGrip(! is_active);
}
