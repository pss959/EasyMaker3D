#include "ViewHandler.h"

#include <assert.h>

#include "Event.h"
#include "Interfaces/IRenderer.h"

using ion::math::Point2i;
using ion::math::Range2i;
using ion::math::Vector2i;

ViewHandler::ViewHandler() {
}

ViewHandler::~ViewHandler() {
}

bool ViewHandler::HandleEvent(const Event &event) {
    // Handle kMouse3 buttons to rotate the view.
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.button == Event::Button::kMouse3) {
        is_changing_view_ = true;
        assert(event.flags.Has(Event::Flag::kPosition2D));
        start_pos_ = event.position2D;
    }
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.button == Event::Button::kMouse3) {
        is_changing_view_ = false;
    }

    // Process new mouse position if in the middle of a drag.
    if (is_changing_view_ && event.device == Event::Device::kMouse &&
        event.flags.Has(Event::Flag::kPosition2D)) {
    }

    return false;
}
