#include "Panels/TextToolPanel.h"

#include "Math/TextUtils.h"
#include "Panes/DropdownPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"

void TextToolPanel::InitInterface() {
    auto &root_pane = GetPane();
    text_pane_    = root_pane->FindTypedSubPane<TextInputPane>("Text");
    font_pane_    = root_pane->FindTypedSubPane<DropdownPane>("Font");
    auto spp      = root_pane->FindTypedSubPane<ContainerPane>("Spacing");
    spacing_pane_ = spp->FindTypedSubPane<SliderPane>("Slider");
    display_pane_ = root_pane->FindTypedSubPane<TextPane>("Display");
    message_pane_ = root_pane->FindTypedSubPane<TextPane>("Message");

    // Set up font choices in the dropdown.
    const StrVec font_names = GetAvailableFontNames();
    font_pane_->SetChoices(font_names, 0);

    // Set up interaction.
    text_pane_->SetValidationFunc(
        [&](const Str &str){ return ValidateText_(str); });
    font_pane_->GetChoiceChanged().AddObserver(
        this, [&](const Str &choice){ ChangeFont_(choice); });
    spacing_pane_->GetValueChanged().AddObserver(
        this, [&](float val){ ChangeSpacing_(val); });

    AddButtonFunc("Apply",
                  [&](){ ReportChange("Apply", InteractionType::kImmediate); });
}

void TextToolPanel::UpdateInterface() {
    message_pane_->SetText("");
    EnableButton("Apply", false);  // Wait for a change.
}

void TextToolPanel::SetValues(const Str &text, const Str &font_name,
                              float char_spacing) {
    // Update each Pane.
    text_pane_->SetInitialText(text);
    font_pane_->SetChoiceFromString(font_name);
    spacing_pane_->SetValue(char_spacing);

    // Update the display.
    display_pane_->SetText(text);
    display_pane_->SetCharacterSpacing(char_spacing);
    display_pane_->SetFontName(font_name);

    // Save the values to be able to detect changes.
    initial_text_      = text;
    initial_font_name_ = font_name;
    initial_spacing_   = char_spacing;
}

Str TextToolPanel::GetTextString() const {
    return text_pane_->GetText();
}

const Str & TextToolPanel::GetFontName() const {
    return font_pane_->GetChoice();
}

float TextToolPanel::GetCharSpacing() const {
    return spacing_pane_->GetValue();
}

bool TextToolPanel::ValidateText_(const Str &text) {
    display_pane_->SetText(text);
    Str reason;
    const bool ok = IsValidStringForFont(GetFontName(), text, reason);
    message_pane_->SetText(ok ? "" : reason);
    // Do this last so that errors can be detected.
    UpdateButton_();
    return ok;
}

void TextToolPanel::ChangeSpacing_(float spacing) {
    display_pane_->SetCharacterSpacing(spacing);
    UpdateButton_();
}

void TextToolPanel::ChangeFont_(const Str &font_name) {
    display_pane_->SetFontName(font_name);
    UpdateButton_();
}

void TextToolPanel::UpdateButton_() {
    // Enable the Apply button if anything has changed and there are no errors.
    EnableButton("Apply",
                 message_pane_->GetText().empty() &&
                 (GetTextString()  != initial_text_ ||
                  GetFontName()    != initial_font_name_ ||
                  GetCharSpacing() != initial_spacing_));
}
