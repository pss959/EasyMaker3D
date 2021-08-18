#include "ViewHandler.h"

#include <assert.h>

#include <ion/math/angle.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>

#include "Event.h"
#include "SG/Scene.h"
#include "View.h"

using ion::math::Anglef;
using ion::math::Rotationf;
using ion::math::Vector2f;

ViewHandler::ViewHandler(View &view) : view_(view) {
}

ViewHandler::~ViewHandler() {
}

bool ViewHandler::HandleEvent(const Event &event) {
    bool handled = false;

    // Handle kMouse3 buttons to rotate the view.
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.button == Event::Button::kMouse3) {
        is_changing_view_ = true;
        assert(event.flags.Has(Event::Flag::kPosition2D));
        start_pos_ = event.position2D;
        start_rot_ = rotation_;
        handled = true;
    }
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.button == Event::Button::kMouse3) {
        is_changing_view_ = false;
        handled = true;
    }

    // Process new mouse position if in the middle of a drag.
    if (is_changing_view_ && event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {

        // Compute the new rotation based on the change in position.
        const Vector2f diff = event.position2D - start_pos_;
        const Anglef yaw   =  Anglef::FromRadians(diff[0]);
        const Anglef pitch = -Anglef::FromRadians(diff[1]);
        const Anglef roll;
        rotation_ = start_rot_ * Rotationf::FromYawPitchRoll(yaw, pitch, roll);

        // Create a copy of the Scene's camera and modify its orientation. Use
        // that to update the View.
        /* XXXX Need to copy Camera here...
        const SG::Camera &camera = *view_.GetScene()->GetCamera();
        camera.orientation = camera.orientation * rotation_;
        view_.UpdateFromCamera(camera);
        */

        handled = true;
    }

    // Shift-Ctrl-R key: Reset the view.
    if (event.flags.Has(Event::Flag::kKeyPress) &&
        event.key_string == "<Shift><Ctrl>r") {
        rotation_ = Rotationf();
        view_.UpdateFromCamera(*view_.GetScene()->GetCamera());
        return true;
    }

    return handled;
}
