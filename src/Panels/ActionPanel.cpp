//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panels/ActionPanel.h"

#include "Base/ActionMap.h"
#include "Panes/ButtonPane.h"
#include "Panes/IconPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/TextPane.h"
#include "Place/ClickInfo.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Widgets/PushButtonWidget.h"

void ActionPanel::SetAction(Action action) {
    if (current_action_ != action) {
        if (! button_map_.empty()) {
            ChangeHighlight_(current_action_, false);
            ChangeHighlight_(action,          true);
        }
        current_action_ = action;
    }
}

void ActionPanel::InitInterface() {
    auto &root_pane = GetPane();
    auto header_pane = root_pane->FindTypedSubPane<TextPane>("CategoryHeader");
    auto button_pane = root_pane->FindTypedSubPane<ButtonPane>("ActionButton");
    auto contents_pane = root_pane->FindTypedSubPane<ContainerPane>("Contents");

    Pane::PaneVec panes;
    ActionMap action_map;
    for (auto cat: Util::EnumValues<ActionCategory>()) {
        auto header = header_pane->CloneTyped<TextPane>(true);
        header->SetText(Util::EnumToWords(cat));
        panes.push_back(header);

        for (auto action: action_map.GetActionsInCategory(cat)) {
            auto but = button_pane->CloneTyped<ButtonPane>(
                true, Util::EnumToWord(action));
            auto icon = but->FindTypedSubPane<IconPane>("Icon");
            auto text = but->FindTypedSubPane<TextPane>("Text");
            icon->SetIconName("MI" + Util::EnumToWord(action));
            text->SetText(Util::EnumToWords(action));
            but->GetButton().GetClicked().AddObserver(
                this, [&, action](const ClickInfo &){ SetAction(action); });
            panes.push_back(but);

            button_map_[action] = but;
        }
    }

    contents_pane->ReplacePanes(panes);

    // Turn off the template panes.
    header_pane->SetEnabled(false);
    button_pane->SetEnabled(false);

    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ Close("Accept"); });
}

void ActionPanel::UpdateInterface() {
    // Highlight the current button.
    ChangeHighlight_(current_action_, true);

    SetFocus("Cancel");
}

void ActionPanel::ChangeHighlight_(Action action, bool state) {
    ASSERT(button_map_.contains(action));
    button_map_[action]->GetButton().SetToggleState(state);

    // Scroll to the highlighted button.
    if (state) {
        auto sp = GetPane()->FindTypedSubPane<ScrollingPane>("ScrollingPane");
        sp->ScrollToShowSubPane(*button_map_[action]);
    }
}
