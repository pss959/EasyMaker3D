#include "Panels/DialogPanel.h"

#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"
#include "SG/Search.h"
#include "Util/Assert.h"

void DialogPanel::SetMessage(const Str &msg) {
    message_->SetText(msg);
}

void DialogPanel::SetSingleResponse(const Str &text) {
    text0_->SetText(text);
    button0_->SetEnabled(true);
    button1_->SetEnabled(false);
    SetFocus(button0_);
}

void DialogPanel::SetChoiceResponse(const Str &text0, const Str &text1,
                                    bool focus_first) {
    text0_->SetText(text0);
    text1_->SetText(text1);
    button0_->SetEnabled(true);
    button1_->SetEnabled(true);
    SetFocus(focus_first ? button0_ : button1_);
}

void DialogPanel::InitInterface() {
    auto &root_pane = *GetPane();
    message_ = root_pane.FindTypedSubPane<TextPane>("Message");
    button0_ = root_pane.FindTypedSubPane<ButtonPane>("Button0");
    button1_ = root_pane.FindTypedSubPane<ButtonPane>("Button1");
    text0_   = button0_->FindTypedSubPane<TextPane>("ButtonText");
    text1_   = button1_->FindTypedSubPane<TextPane>("ButtonText");

    AddButtonFunc("Button0", [&](){ Close(text0_->GetText()); });
    AddButtonFunc("Button1", [&](){ Close(text1_->GetText()); });
}

void DialogPanel::UpdateInterface() {
    // Always recompute the layout size, since it may be smaller now.
    GetSizeChanged().Notify();
}
