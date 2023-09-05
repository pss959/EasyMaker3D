#include "Panels/NameToolPanel.h"

#include "Agents/NameAgent.h"
#include "Models/Model.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"

void NameToolPanel::SetName(const Str &name) {
    input_pane_->SetInitialText(name);
    original_name_ = name;
}

Str NameToolPanel::GetName() const {
    return input_pane_->GetText();
}

void NameToolPanel::InitInterface() {
    auto &root_pane = GetPane();
    input_pane_   = root_pane->FindTypedSubPane<TextInputPane>("Input");
    message_pane_ = root_pane->FindTypedSubPane<TextPane>("Message");

    // Set up validation of Model names.
    auto validate = [&](const Str &name){ return ValidateName_(name); };
    input_pane_->SetValidationFunc(validate);

    AddButtonFunc("Apply",
                  [&](){ ReportChange("Name", InteractionType::kImmediate); });

    // Don't enable the Apply button until a change is made.
    EnableButton("Apply", false);
}

void NameToolPanel::UpdateInterface() {
    message_pane_->SetText("");
}

bool NameToolPanel::ValidateName_(const Str &name) {
    const bool is_changed = name != original_name_;
    bool       is_valid = true;
    Str        msg;

    if (! Model::IsValidName(name)) {
        is_valid = false;
        msg = "Invalid name for Model";
    }
    else if (is_changed && GetContext().name_agent->Find(name)) {
        is_valid = false;
        msg = "Name is in use by another Model";
    }

    message_pane_->SetText(msg);

    EnableButton("Apply", is_valid && is_changed);

    return is_valid;
}
