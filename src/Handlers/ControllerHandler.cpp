//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Handlers/ControllerHandler.h"

#include <ion/math/angleutils.h>

#include "Base/Event.h"
#include "Items/Controller.h"
#include "Items/RadialMenu.h"
#include "Math/Linear.h"
#include "Math/Types.h"

Handler::HandleCode ControllerHandler::HandleEvent(const Event &event) {
    Handler::HandleCode code = HandleCode::kNotHandled;

    if ((event.device == Event::Device::kLeftController ||
         event.device == Event::Device::kRightController) ) {

        // Position and orientation of the Controller. Let other handlers see
        // these as well.
        if (event.flags.Has(Event::Flag::kPosition3D) &&
            event.flags.Has(Event::Flag::kOrientation)) {
            UpdateController_(event);
            code = HandleCode::kHandledContinue;
        }

        // Trap menu events.
        if (UpdateRadialMenu_(event))
            code = HandleCode::kHandledStop;
    }
    return code;
}

void ControllerHandler::UpdateController_(const Event &event) {
    ASSERT(l_controller_ && r_controller_);
    const bool is_left = event.device == Event::Device::kLeftController;
    Controller &controller = is_left ? *l_controller_   : *r_controller_;

    // If the position is the origin, the controller is not active, so hide the
    // model.
    const bool is_active = event.position3D != Point3f::Zero();
    controller.SetEnabled(is_active);
    if (is_active) {
        controller.TranslateTo(event.position3D);
        controller.SetRotation(event.orientation);
    }
}

bool ControllerHandler::UpdateRadialMenu_(const Event &event) {
    ASSERT(l_radial_menu_ && r_radial_menu_);
    RadialMenu &menu = event.device == Event::Device::kLeftController ?
        *l_radial_menu_ : *r_radial_menu_;

    if (menu.IsEnabled()) {
        if (event.flags.Has(Event::Flag::kPosition2D)) {
            const Anglef angle = ion::math::ArcTangent2(event.position2D[1],
                                                        event.position2D[0]);
            menu.HighlightButton(NormalizedAngle(angle));
        }
        else {
            menu.ClearHighlightedButton();
        }

        // Trackpad clicks activate the menu.
        if (event.flags.Has(Event::Flag::kButtonPress) &&
            event.IsTrackpadButton(event.button)) {
            menu.SimulateButtonPress();
            return true;
        }
    }
    return false;
}
