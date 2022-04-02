#include "Panels/TextToolPanel.h"

#include "Math/TextUtils.h"
#include "Panes/DropdownPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"

void TextToolPanel::CreationDone() {
    ToolPanel::CreationDone();

    auto &root_pane = GetPane();
    text_pane_    = root_pane->FindTypedPane<TextInputPane>("Text");
    message_pane_ = root_pane->FindTypedPane<TextPane>("Message");
    font_pane_    = root_pane->FindTypedPane<DropdownPane>("Font");
    auto spp      = root_pane->FindTypedPane<ContainerPane>("Spacing");
    spacing_pane_ = spp->FindTypedPane<SliderPane>("Slider");

    // Set up font choices in the dropdown.
    std::vector<std::string> font_names = GetAvailableFontNames();
    font_pane_->SetChoices(font_names, 0);
}

void TextToolPanel::InitInterface() {
    // XXXX AddButtonFunc("Apply",
    // XXXX    [&](){ ReportChange("Text", InteractionType::kImmediate); });
}

void TextToolPanel::UpdateInterface() {
    message_pane_->SetText("");
}

void TextToolPanel::SetTextString(const std::string &text) {
    text_pane_->SetInitialText(text);
    original_text_ = text;
}

std::string TextToolPanel::GetTextString() const {
    return text_pane_->GetText();
}
