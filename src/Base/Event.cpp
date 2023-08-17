#include "Base/Event.h"

#include <cctype>

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

Str Event::GetKeyString() const {
    Str s;
    if (device == Device::kKeyboard &&
        (flags.Has(Event::Flag::kKeyPress) ||
         flags.Has(Event::Flag::kKeyRelease))) {
        s = BuildKeyString(modifiers, key_name);
    }
    return s;
}

bool Event::ParseKeyString(const Str &key_string, Modifiers &modifiers,
                           Str &key_name, Str &error) {
    using ion::base::CompareCaseInsensitive;
    using ion::base::SplitString;

    modifiers.SetAll(false);
    key_name.clear();
    error.clear();

    // Split the string into parts by "-".
    const StrVec parts = SplitString(key_string, "-");

    // All but the last part should be a valid modifier.
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        const Str &mod = parts[i];
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
    if (parts.empty()) {
        error = "Missing key name";
        return false;
    }
    key_name = ion::base::TrimStartAndEndWhitespace(parts.back());
    if (key_name.empty()) {
        error = "Missing key name";
        return false;
    }
    return true;
}

Str Event::BuildKeyText(const Modifiers &modifiers, const Str &key_name) {
    Str text;

    // Control and Alt cannot be present for regular text, and the key must
    // have a known name,
    if (modifiers.Has(ModifierKey::kControl) ||
        modifiers.Has(ModifierKey::kAlt))
        return text;

    // Special case for the space key.
    if (key_name == "Space") {
        text = " ";
    }
    // Add text only if a single character.
    else if (key_name.size() == 1U) {
        text = key_name;

        // Handle some shifted keys.
        if (modifiers.Has(ModifierKey::kShift)) {
            char &c = text[0];
            if (c >= 'a' && c <= 'z') {
                c = std::toupper(c);
            }
            else {
                // Handle other special characters.
                switch (c) {
                  case ',':  c = '<'; break;
                  case '-':  c = '_'; break;
                  case '.':  c = '>'; break;
                  case '/':  c = '?'; break;
                  case '0':  c = ')'; break;
                  case '1':  c = '!'; break;
                  case '2':  c = '@'; break;
                  case '3':  c = '#'; break;
                  case '4':  c = '$'; break;
                  case '5':  c = '%'; break;
                  case '6':  c = '^'; break;
                  case '7':  c = '&'; break;
                  case '8':  c = '*'; break;
                  case '9':  c = '('; break;
                  case ';':  c = ':'; break;
                  case '=':  c = '+'; break;
                  case '[':  c = '{'; break;
                  case '\'': c = '"'; break;
                  case '\\': c = '|'; break;
                  case ']':  c = '}'; break;
                  case '`':  c = '~'; break;
                  default:            break;
                }
            }
        }
    }
    return text;
}

Str Event::BuildKeyString(const Modifiers &modifiers, const Str &key_name) {
    Str s;
    if (modifiers.Has(ModifierKey::kShift))
        s+= "Shift-";
    if (modifiers.Has(ModifierKey::kControl))
        s += "Ctrl-";
    if (modifiers.Has(ModifierKey::kAlt))
        s += "Alt-";
    s += key_name;
    return s;
}

Str Event::GetControllerButtonString() const {
    Str s;
    if ((device == Device::kLeftController ||
         device == Device::kRightController) &&
        (flags.Has(Event::Flag::kButtonPress) ||
         flags.Has(Event::Flag::kButtonRelease))) {
        s += device == Device::kLeftController ? "L:" : "R:";
        s += Util::EnumToWord(button);
    }
    return s;
}

// LCOV_EXCL_START
Str Event::ToString() const {
    Str s = "=== Event [" + Util::ToString(serial, 5) +
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
// LCOV_EXCL_STOP
