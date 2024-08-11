//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/TextKeyPane.h"

#include "Panes/TextPane.h"

void TextKeyPane::AddFields() {
    AddField(text_.Init("text"));
    AddField(shifted_text_.Init("shifted_text"));
    AddField(label_.Init("label"));

    KeyPane::AddFields();
}

bool TextKeyPane::IsValid(Str &details) {
    if (! KeyPane::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
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
        text_pane_ = FindTypedSubPane<TextPane>("Text");
        ProcessShift(false);
    }
}

TextAction TextKeyPane::GetAction(Str &chars) const {
    chars = GetText_();
    return TextAction::kInsert;
}

void TextKeyPane::ProcessShift(bool is_shifted) {
    const Str &label = label_;
    if (! label.empty())
        text_pane_->SetText(label);
    else
        text_pane_->SetText(is_shifted ? shifted_text_ : text_);
}

Str TextKeyPane::GetText_() const {
    return text_pane_->GetText();
}
