#include "Panels/RadialMenuPanel.h"

#include <vector>

#include "Panes/RadioButtonPane.h"
#include "Settings.h"

void RadialMenuPanel::InitInterface() {
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptEdits_(); });
}

void RadialMenuPanel::UpdateInterface() {
    auto &root_pane = GetPane();

    // Set up mode radio buttons.
    std::vector<RadioButtonPanePtr> mode_buttons;
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode0"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode1"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode2"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode3"));
    RadioButtonPane::CreateGroup(mode_buttons, 0);

    // XXXX
}

void RadialMenuPanel::AcceptEdits_() {
    // XXXX
}
