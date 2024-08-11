//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Panels/ToolPanelTestBase.h"

#include "Panels/ToolPanel.h"

void ToolPanelTestBase::ObserveChanges(ToolPanel &panel) {
    // Set up an Observer to detect Panel changes.
    auto func = [&](const Str &name, ToolPanel::InteractionType type){
        ++last_change_info_.count;
        last_change_info_.name = name;
        last_change_info_.type = Util::EnumName(type);
    };
    panel.GetInteraction().AddObserver("key", func);
}
