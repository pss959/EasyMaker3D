#include "Handlers/ControllerHandler.h"

#include <ion/math/angleutils.h>

#include "Base/Event.h"
#include "Items/Controller.h"
#include "Items/RadialMenu.h"
#include "Math/Types.h"

bool ControllerHandler::HandleEvent(const Event &event) {
    if ((event.device == Event::Device::kLeftController ||
         event.device == Event::Device::kRightController) ) {

        // Position and orientation of the Controller.
        if (event.flags.Has(Event::Flag::kPosition3D) &&
            event.flags.Has(Event::Flag::kOrientation))
            UpdateController_(event);

        if (UpdateRadialMenu_(event))
            return true;
    }

    // No need to trap non-menu events - others may be interested.
    return false;
}

void ControllerHandler::UpdateController_(const Event &event) {
    Controller &controller = event.device == Event::Device::kLeftController ?
        *l_controller_ : *r_controller_;

    // If the position is the origin, the controller is not active, so
    // hide the model.
    const bool is_active = event.position3D != Point3f::Zero();
    controller.SetEnabled(is_active);
    if (is_active) {
        controller.SetTranslation(event.position3D);
        controller.SetRotation(event.orientation);
    }
}

bool ControllerHandler::UpdateRadialMenu_(const Event &event) {
    RadialMenu &menu = event.device == Event::Device::kLeftController ?
        *l_radial_menu_ : *r_radial_menu_;

    if (menu.IsEnabled()) {
        if (event.flags.Has(Event::Flag::kPosition2D)) {
            const Anglef angle = ion::math::ArcTangent2(event.position2D[1],
                                                        event.position2D[0]);
            menu.HighlightButton(angle);
        }
        else {
            menu.ClearHighlightedButton();
        }

        // Directional pad or joystick clicks activate the menu.
        if (event.flags.Has(Event::Flag::kButtonPress) &&
            (event.button == Event::Button::kCenter ||
             event.button == Event::Button::kLeft   ||
             event.button == Event::Button::kRight  ||
             event.button == Event::Button::kUp     ||
             event.button == Event::Button::kDown)) {
            menu.SimulateButtonPress();
            return true;
        }
    }
    return false;
}
