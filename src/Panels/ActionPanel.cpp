#include "Panels/ActionPanel.h"

#include "ActionMap.h"
#include "Managers/ColorManager.h"
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

    current_button_.reset();

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
                current_button_ = but;
            }
            else {
                text->SetText(Util::EnumToWords(action));
            }

            but->GetButton().GetClicked().AddObserver(
                this, [&, action, but](const ClickInfo &){
                    ButtonClicked_(action, but); });

            panes.push_back(but);
        }
    }

    contents_pane->ReplacePanes(panes);

    // Now that the panes have been added, it should be OK to activate the
    // current button.
    if (current_button_)
        current_button_->GetButton().SetToggleState(true);

    // Turn off the template panes.
    header_pane->SetEnabled(false);
    button_pane->SetEnabled(false);

    SetFocus("Cancel");
}

void ActionPanel::ButtonClicked_(Action action, const ButtonPanePtr &but) {
    if (current_button_)
        current_button_->GetButton().SetToggleState(false);

    current_action_ = action;
    current_button_ = but;
    current_button_->GetButton().SetToggleState(true);
}

void ActionPanel::Accept_() {
    // XXXX Do something.
}
