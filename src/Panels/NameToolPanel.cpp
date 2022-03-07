#include "Panels/NameToolPanel.h"

void NameToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    input_pane_   = root_pane->FindTypedPane<TextInputPane>("Input");
    message_pane_ = root_pane->FindTypedPane<TextPane>("Message");

    // XXXX Set up validation; need NameManager.
    // XXXX Also need to know when panel is closed to execute command.

    message_pane_->SetText("");
}

void NameToolPanel::SetName(const std::string &name) {
    input_pane_->SetInitialText(name);
}

std::string NameToolPanel::GetName() const {
    return input_pane_->GetText();
}
