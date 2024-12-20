//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Handlers/ViewHandler.h"

#include "Base/Event.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"

using ion::math::Anglef;
using ion::math::Rotationf;
using ion::math::Vector2f;

Handler::HandleCode ViewHandler::HandleEvent(const Event &event) {
    bool handled = false;

    // Handle kMouse3 buttons to rotate the view.
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.button == Event::Button::kMouse3) {
        is_changing_view_ = true;
        ASSERT(event.flags.Has(Event::Flag::kPosition2D));
        start_pos_ = event.position2D;
        start_rot_ = rotation_ = camera_->GetOrientation();
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

        if (! is_fixed_pos_)
            SetPosition_();

        handled = true;
    }

    // Ctrl-Period key: Reset the view.
    if (event.flags.Has(Event::Flag::kKeyPress) &&
        event.GetKeyString() == "Ctrl-.") {
        ResetView();
        handled = true;
    }

    return handled ? HandleCode::kHandledStop : HandleCode::kNotHandled;
}

void ViewHandler::SetRotationCenter(const Point3f &center) {
    rot_center_ = center;
}

void ViewHandler::ResetView() {
    rotation_ = Rotationf::Identity();
    camera_->SetOrientation(rotation_);
    if (! is_fixed_pos_)
        SetPosition_();
}

void ViewHandler::SetPosition_() {
    const float rad = ion::math::Length(rot_center_ - camera_->GetPosition());
    camera_->SetPosition(rot_center_ - rad * camera_->GetViewDirection());
}
