#include "Panels/ActionPanel.h"

#include "ActionMap.h"
#include "Managers/ColorManager.h"
#include "Panes/ButtonPane.h"
#include "Panes/IconPane.h"
#include "Panes/TextPane.h"
#include "Util/Enum.h"

void ActionPanel::InitInterface() {
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ Accept_(); });
}

void ActionPanel::UpdateInterface() {
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

            if (action == current_action_) {
                text->SetColor(
                    ColorManager::GetSpecialColor("FileHighlightColor"));
                text->SetText(Util::EnumToWords(action) + " [CURRENT]");
            }
            else {
                text->SetText(Util::EnumToWords(action));
            }
            panes.push_back(but);
        }
    }

    contents_pane->ReplacePanes(panes);

    // Turn off the template panes.
    header_pane->SetEnabled(false);
    button_pane->SetEnabled(false);

    SetFocus("Cancel");
}

void ActionPanel::Accept_() {
    // XXXX Do something.
}
