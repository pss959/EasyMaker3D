#include "Panels/ActionPanel.h"

#include "ActionMap.h"

void ActionPanel::InitInterface() {
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ Accept_(); });
}

void ActionPanel::UpdateInterface() {
    // auto &root_pane = GetPane();
    // auto header_pane = root_pane.FindTypedPane<TextPane>("CategoryHeader");
    // auto button_pane = root_pane.FindTypedPane<ButtonPane>("ActionButton");

    ActionMap action_map;

    // // XXXX


    // // Turn off the template panes.
    // header_pane->SetEnabled(false);
    // button_pane->SetEnabled(false);

    SetFocus("Cancel");
}

void ActionPanel::Accept_() {
    // XXXX Do something.
}
