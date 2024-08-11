//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Handlers/LogHandler.h"

#include "Base/Event.h"
#include "Util/General.h"

LogHandler::LogHandler() {
    SetEnabled(false);
}

LogHandler::~LogHandler() {
}

Handler::HandleCode LogHandler::HandleEvent(const Event &event) {
    if (IsEnabled() && PassesFilters_(event))
        std::cout << event.ToString() << "\n";
    return HandleCode::kNotHandled;
}

bool LogHandler::PassesFilters_(const Event &event) const {
    if (! devices_.empty() && ! Util::Contains(devices_, event.device))
        return false;

    if (flags_.HasAny() && ! flags_.HasAnyFrom(event.flags))
        return false;

    return true;
}
