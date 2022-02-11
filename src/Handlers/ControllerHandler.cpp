#include "ControllerHandler.h"

#include "Event.h"
#include "Math/Types.h"
#include "SG/Node.h"

bool ControllerHandler::HandleEvent(const Event &event) {
    if ((event.device == Event::Device::kLeftController ||
         event.device == Event::Device::kRightController) &&
        event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        ControllerPtr controller =
            event.device == Event::Device::kLeftController ?
            l_controller_ : r_controller_;
        if (event.orientation.IsIdentity()) {
            // If the orientation is identity, the controller is not active, so
            // hide the model.
            controller->SetEnabled(false);
        }
        else {
            controller->SetEnabled(true);
            controller->SetTranslation(event.position3D);
            controller->SetRotation(event.orientation);
        }
    }
    // No need to trap these events - others may be interested.
    return false;
}
