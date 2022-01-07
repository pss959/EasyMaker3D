#include "Panels/DialogPanel.h"

#include "SG/Search.h"
#include "Util/Assert.h"

void DialogPanel::SetMessage(const std::string &msg) {
    message_->SetText(msg);
}

void DialogPanel::SetSingleResponse(const std::string &text) {
    text0_->SetText(text);
    button0_->SetEnabled(SG::Node::Flag::kTraversal, true);
    button1_->SetEnabled(SG::Node::Flag::kTraversal, false);
}

void DialogPanel::SetChoiceResponse(const std::string &text0,
                                    const std::string &text1) {
    text0_->SetText(text0);
    text1_->SetText(text1);
    button0_->SetEnabled(SG::Node::Flag::kTraversal, true);
    button1_->SetEnabled(SG::Node::Flag::kTraversal, true);
}

void DialogPanel::InitInterface() {
    auto &root_pane = *GetPane();
    message_ = root_pane.FindTypedPane<TextPane>("Message");
    button0_ = root_pane.FindTypedPane<ButtonPane>("Button0");
    button1_ = root_pane.FindTypedPane<ButtonPane>("Button1");
    text0_   = button0_->FindTypedPane<TextPane>("ButtonText");
    text1_   = button1_->FindTypedPane<TextPane>("ButtonText");
}
