#include "Panes/TextKeyPane.h"

#include "Panes/TextPane.h"

void TextKeyPane::AddFields() {
    AddField(text_);
    AddField(shifted_text_);
    AddField(label_);
    KeyPane::AddFields();
}

bool TextKeyPane::IsValid(std::string &details) {
    if (! KeyPane::IsValid(details))
        return false;

    if (text_.GetValue().empty()) {
        details = "No text specified";
        return false;
    }
    if (shifted_text_.GetValue().empty()) {
        details = "No shifted text specified";
        return false;
    }

    return true;
}

void TextKeyPane::CreationDone() {
    KeyPane::CreationDone();

    if (! IsTemplate()) {
        text_pane_ = FindTypedPane<TextPane>("Text");
        ProcessShift(false);
    }
}

TextAction TextKeyPane::GetAction(std::string &chars) const {
    chars = GetText_();
    return TextAction::kInsert;
}

void TextKeyPane::ProcessShift(bool is_shifted) {
    const std::string &label = label_;
    if (! label.empty())
        text_pane_->SetText(label);
    else
        text_pane_->SetText(is_shifted ? shifted_text_ : text_);
}

std::string TextKeyPane::GetText_() const {
    return text_pane_->GetText();
}
