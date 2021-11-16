#pragma once

#include "Enums/Action.h"

/// The RadialMenuInfo struct packages up information about a radial menu.
struct RadialMenuInfo {
    /// Enum indicating the number of buttons in the menu.
    enum class Count {
        kCount1 = 1,
        kCount2 = 2,
        kCount4 = 4,
        kCount8 = 8,
    };

    static const int kMaxCount = 8;  ///< Maximum number of buttons.

    /// Count of visible buttons.
    Count count;

    /// Current Action for each menu button.
    std::vector<Action> actions{kMaxCount, Action::kNone};
};
