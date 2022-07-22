#include "Panels/NameToolPanel.h"

#include "Managers/NameManager.h"
#include "Models/Model.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"

void NameToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    input_pane_   = root_pane->FindTypedPane<TextInputPane>("Input");
    message_pane_ = root_pane->FindTypedPane<TextPane>("Message");

    // Set up validation of Model names.
    auto validate = [&](const std::string &name){ return ValidateName_(name); };
    input_pane_->SetValidationFunc(validate);
}

void NameToolPanel::InitInterface() {
    AddButtonFunc("Apply",
                  [&](){ ReportChange("Name", InteractionType::kImmediate); });

    // Don't enable the Apply button until a change is made.
    EnableButton("Apply", false);
}

void NameToolPanel::UpdateInterface() {
    message_pane_->SetText("");
}

void NameToolPanel::SetName(const std::string &name) {
    input_pane_->SetInitialText(name);
    original_name_ = name;
}

std::string NameToolPanel::GetName() const {
    return input_pane_->GetText();
}

bool NameToolPanel::ValidateName_(const std::string &name) {
    const bool  is_changed = name != original_name_;
    bool        is_valid = true;
    std::string msg;

    if (! Model::IsValidName(name)) {
        is_valid = false;
        msg = "Invalid name for Model";
    }
    else if (is_changed && GetContext().name_manager->Find(name)) {
        is_valid = false;
        msg = "Name is in use by another Model";
    }

    message_pane_->SetText(msg);

    EnableButton("Apply", is_valid && is_changed);

    return is_valid;
}
