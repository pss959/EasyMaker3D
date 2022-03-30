#include "Panels/TextToolPanel.h"

#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"

void TextToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    input_pane_   = root_pane->FindTypedPane<TextInputPane>("TextInput");
    message_pane_ = root_pane->FindTypedPane<TextPane>("Message");

    // XXXX Dropdown for font
    // XXXX Slider for char spacing.
}

void TextToolPanel::InitInterface() {
    // XXXX AddButtonFunc("Apply",
    // XXXX    [&](){ ReportChange("Text", InteractionType::kImmediate); });
}

void TextToolPanel::UpdateInterface() {
    message_pane_->SetText("");
}

void TextToolPanel::SetTextString(const std::string &text) {
    input_pane_->SetInitialText(text);
    original_text_ = text;
}

std::string TextToolPanel::GetTextString() const {
    return input_pane_->GetText();
}
