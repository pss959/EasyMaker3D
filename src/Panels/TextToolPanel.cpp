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
    font_pane_    = root_pane->FindTypedPane<DropdownPane>("Font");
    auto spp      = root_pane->FindTypedPane<ContainerPane>("Spacing");
    spacing_pane_ = spp->FindTypedPane<SliderPane>("Slider");
    display_pane_ = root_pane->FindTypedPane<TextPane>("Display");
    message_pane_ = root_pane->FindTypedPane<TextPane>("Message");

    // Set up font choices in the dropdown.
    std::vector<std::string> font_names = GetAvailableFontNames();
    font_pane_->SetChoices(font_names, 0);

    // Set up interaction.
    text_pane_->SetValidationFunc(
        [&](const std::string &str){ return ValidateText_(str); });
    font_pane_->GetChoiceChanged().AddObserver(
        this, [&](const std::string &choice){ ChangeFont_(choice); });
    spacing_pane_->GetValueChanged().AddObserver(
        this, [&](float val){ ChangeSpacing_(val); });
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
    display_pane_->SetText(text);
    original_text_ = text;
}

std::string TextToolPanel::GetTextString() const {
    return text_pane_->GetText();
}

bool TextToolPanel::ValidateText_(const std::string &text) {
    display_pane_->SetText(text);
    return true;   // XXXX Any errors???
}

void TextToolPanel::ChangeSpacing_(float spacing) {
    display_pane_->SetCharacterSpacing(spacing);
}

void TextToolPanel::ChangeFont_(const std::string &font_name) {
    display_pane_->SetFontName(font_name);
}
