#include "LogHandler.h"

#include "Event.h"
#include "Util/Enum.h"
#include "Util/Flags.h"
#include "Util/General.h"

LogHandler::LogHandler() {
}

LogHandler::~LogHandler() {
}

bool LogHandler::HandleEvent(const Event &event) {
    // Use Ctrl-L to toggle event logging.
    // TODO: Move this somewhere better?
    if (event.flags.Has(Event::Flag::kKeyPress) &&
        event.key_string == "<Ctrl>l") {
        SetEnabled(! IsEnabled());
    }

    if (IsEnabled() && PassesFilters_(event)) {
        std::cout << "=== Event: dev=" << Util::EnumName(event.device)
                  << " flags=" << event.flags.ToString();

        if (event.flags.Has(Event::Flag::kButtonPress) ||
            event.flags.Has(Event::Flag::kButtonRelease))
            std::cout << " but=" << Util::EnumName(event.button);

        if (event.flags.Has(Event::Flag::kKeyPress) ||
            event.flags.Has(Event::Flag::kKeyRelease))
            std::cout << " key='" << event.key_string << "'";

        std::cout << "\n";
    }
    return false;
}

bool LogHandler::PassesFilters_(const Event &event) const {
    if (! devices_.empty() && ! Util::Contains(devices_, event.device))
        return false;

    if (flags_.HasAny() && ! flags_.HasAnyFrom(event.flags))
        return false;

    return true;
}
