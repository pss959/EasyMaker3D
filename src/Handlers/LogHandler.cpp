#include "LogHandler.h"

#include "Event.h"
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

    if (IsEnabled() && PassesFilters_(event))
        std::cout << event.ToString() << "\n";

    return false;
}

bool LogHandler::PassesFilters_(const Event &event) const {
    if (! devices_.empty() && ! Util::Contains(devices_, event.device))
        return false;

    if (flags_.HasAny() && ! flags_.HasAnyFrom(event.flags))
        return false;

    return true;
}
