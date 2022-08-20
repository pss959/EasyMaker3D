#if ENABLE_DEBUG_FEATURES

#include "Handlers/DragRectHandler.h"

#include "Base/Event.h"

bool DragRectHandler::HandleEvent(const Event &event) {
    bool handled = false;
    if (is_dragging_) {
        if (IsDragEnd_(event)) {
            std::cerr << "XXXX DragRectHandler END\n";
            is_dragging_ = false;
            handled = true;
        }
        else if (event.flags.Has(Event::Flag::kPosition2D)) {
            // XXXX
            handled = true;
        }
    }
    else {
        if (IsDragStart_(event)) {
            std::cerr << "XXXX DragRectHandler START\n";
            is_dragging_ = true;
            handled = true;
        }
    }
    return handled;
}

bool DragRectHandler::IsDragStart_(const Event &event) {
    return
        event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == Event::Device::kMouse &&
        event.button == Event::Button::kMouse1 &&
        event.modifiers.Has(Event::ModifierKey::kControl);
}

bool DragRectHandler::IsDragEnd_(const Event &event) {
    return
        event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == Event::Device::kMouse &&
        event.button == Event::Button::kMouse1;
}

#endif
