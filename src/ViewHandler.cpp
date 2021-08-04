#include "ViewHandler.h"

#include <assert.h>

#include <ion/math/angle.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>

#include "Event.h"
#include "View.h"

using ion::math::Anglef;
using ion::math::Rotationf;
using ion::math::RotationMatrixH;
using ion::math::Vector2f;

ViewHandler::ViewHandler() {
}

ViewHandler::~ViewHandler() {
}

bool ViewHandler::HandleEvent(const Event &event) {
    if (! view_)
        return false;

    bool handled = false;

    // Handle kMouse3 buttons to rotate the view.
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.button == Event::Button::kMouse3) {
        is_changing_view_ = true;
        assert(event.flags.Has(Event::Flag::kPosition2D));
        start_pos_ = event.position2D;
        start_rot_ = view_->camera_rotation;
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

        const Vector2f diff = event.position2D - start_pos_;
        const Anglef yaw   =  Anglef::FromRadians(diff[0]);
        const Anglef pitch = -Anglef::FromRadians(diff[1]);
        const Anglef roll;
        view_->camera_rotation = start_rot_ *
            Rotationf::FromYawPitchRoll(yaw, pitch, roll);

        handled = true;
    }

    return handled;
}
