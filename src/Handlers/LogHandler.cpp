#include "LogHandler.h"

#include "Base/Event.h"
#include "Util/General.h"

LogHandler::LogHandler() {
}

LogHandler::~LogHandler() {
}

bool LogHandler::HandleEvent(const Event &event) {
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
