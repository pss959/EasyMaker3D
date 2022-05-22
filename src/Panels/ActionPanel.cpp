#include "Panels/ActionPanel.h"

#include "App/ClickInfo.h"
#include "Base/ActionMap.h"
#include "Panes/ButtonPane.h"
#include "Panes/IconPane.h"
#include "Panes/TextPane.h"
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
    auto header_pane = root_pane->FindTypedPane<TextPane>("CategoryHeader");
    auto button_pane = root_pane->FindTypedPane<ButtonPane>("ActionButton");
    auto contents_pane = root_pane->FindTypedPane<ContainerPane>("Contents");

    std::vector<PanePtr> panes;
    ActionMap action_map;
    for (auto cat: Util::EnumValues<ActionCategory>()) {
        auto header = header_pane->CloneTyped<TextPane>(true);
        header->SetText(Util::EnumToWords(cat));
        panes.push_back(header);

        for (auto action: action_map.GetActionsInCategory(cat)) {
            auto but = button_pane->CloneTyped<ButtonPane>(true);
            auto icon = but->FindTypedPane<IconPane>("Icon");
            auto text = but->FindTypedPane<TextPane>("Text");
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
    button_map_[action]->GetButton().SetToggleState(state);
}
