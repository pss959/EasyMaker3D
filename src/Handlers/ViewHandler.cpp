#include "ViewHandler.h"

#include "Assert.h"
#include "Event.h"
#include "SG/WindowCamera.h"

using ion::math::Anglef;
using ion::math::Rotationf;
using ion::math::Vector2f;

bool ViewHandler::HandleEvent(const Event &event) {
    bool handled = false;

    // Handle kMouse3 buttons to rotate the view.
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.button == Event::Button::kMouse3) {
        is_changing_view_ = true;
        ASSERT(event.flags.Has(Event::Flag::kPosition2D));
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

        // Modify the camera's orientation.
        camera_->SetOrientation(rotation_);

        handled = true;
    }

    // Ctrl-Period key: Reset the view.
    if (event.flags.Has(Event::Flag::kKeyPress) &&
        event.GetKeyString() == "<Ctrl>.") {
        ResetView();
        return true;
    }

    return handled;
}

void ViewHandler::ResetView() {
    rotation_ = Rotationf::Identity();
    camera_->SetOrientation(rotation_);
}
