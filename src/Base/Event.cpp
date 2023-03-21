#include "Base/Event.h"

#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/String.h"

bool Event::IsTrackpadButton(Button button) {
    return
        button == Event::Button::kCenter ||
        button == Event::Button::kLeft   ||
        button == Event::Button::kRight  ||
        button == Event::Button::kUp     ||
        button == Event::Button::kDown;
}

std::string Event::GetKeyString() const {
    std::string s;
    if (device == Device::kKeyboard &&
        (flags.Has(Event::Flag::kKeyPress) ||
         flags.Has(Event::Flag::kKeyRelease))) {
        if (modifiers.Has(ModifierKey::kShift))
            s+= "Shift-";
        if (modifiers.Has(ModifierKey::kControl))
            s += "Ctrl-";
        if (modifiers.Has(ModifierKey::kAlt))
            s += "Alt-";
        s += key_name;
    }
    return s;
}

std::string Event::GetControllerButtonString() const {
    std::string s;
    if ((device == Device::kLeftController ||
         device == Device::kRightController) &&
        (flags.Has(Event::Flag::kButtonPress) ||
         flags.Has(Event::Flag::kButtonRelease))) {
        s += device == Device::kLeftController ? "L:" : "R:";
        s += Util::EnumToWord(button);
    }
    return s;
}

std::string Event::ToString() const {
    std::string s = "=== Event [" + Util::ToString(serial, 5) +
        "] dev=" + Util::EnumName(device) +
        " flags=" + flags.ToString();

    if (is_modified_mode)
        s += " MOD";

    if (flags.Has(Event::Flag::kButtonPress) ||
        flags.Has(Event::Flag::kButtonRelease))
        s += " but=" + Util::EnumName(button);

    if (flags.Has(Event::Flag::kKeyPress) ||
        flags.Has(Event::Flag::kKeyRelease))
        s += " key='" + GetKeyString() + "'";

    if (modifiers.HasAny())
        s += " mods=" + modifiers.ToString();

    return s;
}
