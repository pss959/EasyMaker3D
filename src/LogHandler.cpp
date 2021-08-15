#include "LogHandler.h"

#include "Event.h"
#include "Util/Enum.h"
#include "Util/General.h"

LogHandler::LogHandler() {
}

LogHandler::~LogHandler() {
}

bool LogHandler::HandleEvent(const Event &event) {
    // Use Ctrl-L to toggle event logging. XXXX
    if (event.flags.Has(Event::Flag::kKeyPress) &&
        event.key_string == "<Ctrl>l") {
        is_enabled_ = ! is_enabled_;
    }

    if (is_enabled_ && PassesFilters_(event)) {
        std::cout << "=== Event: dev=" << Util::EnumName(event.device)
                  << " flags=" << Util::EnumFlagNames(event.flags);

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
