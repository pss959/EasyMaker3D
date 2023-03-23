#include "Base/Event.h"

#include <ion/base/stringutils.h>

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
        s = BuildKeyString(modifiers, key_name);
    }
    return s;
}

bool Event::ParseKeyString(const std::string &key_string,
                           Modifiers &modifiers, std::string &key_name,
                           std::string &error) {
    using ion::base::CompareCaseInsensitive;
    using ion::base::SplitString;

    // Split the string into parts by "-".
    const std::vector<std::string> parts = SplitString(key_string, "-");

    // All but the last part should be a valid modifier.
    modifiers.SetAll(false);
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        const std::string &mod = parts[i];
        if      (CompareCaseInsensitive(mod, "shift") == 0)
            modifiers.Set(Event::ModifierKey::kShift);
        else if (CompareCaseInsensitive(mod, "ctrl") == 0)
            modifiers.Set(Event::ModifierKey::kControl);
        else if (CompareCaseInsensitive(mod, "alt") == 0)
            modifiers.Set(Event::ModifierKey::kAlt);
        else {
            error = "Invalid key modifier: \"" + mod + "\"";
            return false;
        }
    }

    // The last part is assumed to be a valid key name. If it isn't, it won't
    // do anything, but is not really an error. Leaving this name unvalidated
    // makes it possible to handle other (and future) GLFW key names.
    key_name = parts.back();
    return true;
}

std::string Event::BuildKeyString(const Modifiers &modifiers,
                                  const std::string &key_name) {
    std::string s;
    if (modifiers.Has(ModifierKey::kShift))
        s+= "Shift-";
    if (modifiers.Has(ModifierKey::kControl))
        s += "Ctrl-";
    if (modifiers.Has(ModifierKey::kAlt))
        s += "Alt-";
    s += key_name;
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
