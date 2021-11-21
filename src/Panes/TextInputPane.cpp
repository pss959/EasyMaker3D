#include "Panes/TextInputPane.h"

#include <functional>

#include "ClickInfo.h"
#include "Managers/ColorManager.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

void TextInputPane::AddFields() {
    AddField(initial_text_);
    BoxPane::AddFields();
}

void TextInputPane::SetInitialText(const std::string &text) {
    initial_text_ = text;
    if (text_pane_)
        text_pane_->SetText(text);
}

void TextInputPane::PreSetUpIon() {
    BoxPane::PreSetUpIon();

    // XXXX Need this?
}

void TextInputPane::PostSetUpIon() {
    BoxPane::PostSetUpIon();

    // Access and set up the TextPane.
    text_pane_ = SG::FindTypedNodeUnderNode<TextPane>(*this, "Text");
    if (text_pane_->GetText() != initial_text_.GetValue())
        text_pane_->SetText(initial_text_);

    // Set up the PushButtonWidget.
    auto button = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    button->GetClicked().AddObserver(
        this, std::bind(&TextInputPane::ProcessClick_, this,
                        std::placeholders::_1));
}

void TextInputPane::Activate() {
    if (! is_active_) {
        std::cerr << "XXXX Activate " << GetDesc() << "\n";
        is_active_ = true;
        SetBackgroundColor_("ActiveTextInputColor");
        ShowCursor_(true);
    }
}

void TextInputPane::Deactivate() {
    if (is_active_) {
        std::cerr << "XXXX Deactivate " << GetDesc() << "\n";
        is_active_ = false;
        SetBackgroundColor_("InactiveTextInputColor");
        ShowCursor_(false);
    }
}

void TextInputPane::SetBackgroundColor_(const std::string &color_name) {
    auto bg = SG::FindNodeUnderNode(*this, "Background");
    bg->SetBaseColor(ColorManager::GetSpecialColor(color_name));
}

void TextInputPane::ShowCursor_(bool show) {
    auto cursor = SG::FindNodeUnderNode(*this, "Cursor");
    cursor->SetEnabled(SG::Node::Flag::kTraversal, show);
    cursor->SetTranslation(Vector3f(cursor_pos_ * 10, 0, 0)); // XXXX
}

void TextInputPane::ProcessClick_(const ClickInfo &info) {
    // If the pane is already active, adjust the cursor position. Otherwise,
    // just activate it without changing the position.
    if (is_active_) {
        // XXXX Compute cursor position from click.
        std::cerr << "XXXX Adjust cursor position in " << GetDesc() << "\n";
    }
    else {
        Activate();
    }
}
