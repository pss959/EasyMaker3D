#include "Event.h"

#include "Util/Enum.h"
#include "Util/String.h"

std::string Event::ToString() const {
    std::string s = "=== Event: dev=" + Util::EnumName(device) +
        " flags=" + flags.ToString();

    if (flags.Has(Event::Flag::kButtonPress) ||
        flags.Has(Event::Flag::kButtonRelease))
        s += " but=" + Util::EnumName(button);

    if (flags.Has(Event::Flag::kKeyPress) ||
        flags.Has(Event::Flag::kKeyRelease))
        s += " key='" + key_string + "'";

    return s;
}
