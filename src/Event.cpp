#include "Event.h"

#include "Util/Enum.h"
#include "Util/String.h"

std::string Event::GetKeyString() const {
    std::string mods;
    if (modifiers.Has(ModifierKey::kShift))
        mods += "<Shift>";
    if (modifiers.Has(ModifierKey::kControl))
        mods += "<Ctrl>";
    if (modifiers.Has(ModifierKey::kAlt))
        mods += "<Alt>";
    return mods + key_name;
}

std::string Event::ToString() const {
    std::string s = "=== Event: dev=" + Util::EnumName(device) +
        " flags=" + flags.ToString();

    if (flags.Has(Event::Flag::kButtonPress) ||
        flags.Has(Event::Flag::kButtonRelease))
        s += " but=" + Util::EnumName(button);

    if (flags.Has(Event::Flag::kKeyPress) ||
        flags.Has(Event::Flag::kKeyRelease))
        s += " key='" + GetKeyString() + "'";

    return s;
}
