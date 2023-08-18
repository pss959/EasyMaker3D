#include "Panes/SpecialKeyPane.h"

#include "Panes/IconPane.h"
#include "Panes/TextPane.h"

void SpecialKeyPane::AddFields() {
    AddField(action_.Init("action"));
    AddField(icon_name_.Init("icon_name"));
    AddField(label_.Init("label"));

    KeyPane::AddFields();
}

bool SpecialKeyPane::IsValid(Str &details) {
    if (! KeyPane::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    // Either icon_name_ or label_ must be non-empty.
    if (icon_name_.GetValue().empty() && label_.GetValue().empty()) {
        details = "No icon_name or label specified";
        return false;
    }
    // Do not use this class for inserting characters.
    if (action_ == TextAction::kInsert) {
        details = "SpecialKeyPane action should not be kInsert";
        return false;
    }

    return true;
}

void SpecialKeyPane::CreationDone() {
    KeyPane::CreationDone();

    if (! IsTemplate()) {
        // Access the TextPane and IconPane.
        auto text_pane = FindTypedPane<TextPane>("Text");
        auto icon_pane = FindTypedPane<IconPane>("Icon");

        // Disable the IconPane or TextPane depending on whether there is an
        // icon name.
        if (icon_name_.GetValue().empty()) {
            text_pane->SetText(label_);
            icon_pane->SetEnabled(false);
        }
        else {
            icon_pane->SetIconName(icon_name_);
            text_pane->SetEnabled(false);
        }
    }
}
