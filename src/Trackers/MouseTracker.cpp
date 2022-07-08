#include "Trackers/MouseTracker.h"

#include "App/SceneContext.h"
#include "Util/Assert.h"

MouseTracker::MouseTracker(Actuator actuator) : PointerTracker(actuator) {
    ASSERT(actuator == Actuator::kMouse);
}

Event::Device MouseTracker::GetDevice() const {
    return Event::Device::kMouse;
}

bool MouseTracker::GetRay(const Event &event, Ray &ray) {
    if (event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {
        ray = GetContext().frustum.BuildRay(event.position2D);
        return true;
    }
    return false;
}
