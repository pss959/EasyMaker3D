#include "Panes/TextInputPane.h"

#include <functional>

#include "ClickInfo.h"
#include "Event.h"
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

    if (IsTemplate())
        return;

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
        is_active_ = true;

        UpdateCharWidth_();

        SetBackgroundColor_("ActiveTextInputColor");
        ShowCursor_(true);
    }
}

void TextInputPane::Deactivate() {
    if (is_active_) {
        is_active_ = false;
        SetBackgroundColor_("InactiveTextInputColor");
        ShowCursor_(false);
    }
}

bool TextInputPane::HandleEvent(const Event &event) {
    if (! is_active_)
        return false;

    if (event.flags.Has(Event::Flag::kKeyPress)) {
        if (event.key_string == "Right") {
            if (cursor_pos_ < text_pane_->GetText().size())
                MoveCursor_(cursor_pos_ + 1);
        }
        else if (event.key_string == "Left") {
            if (cursor_pos_ > 0)
                MoveCursor_(cursor_pos_ - 1);
        }
        else if (event.key_string == "Up") {
            MoveCursor_(0);
        }
        else if (event.key_string == "Down") {
            MoveCursor_(text_pane_->GetText().size());
        }
        else {
            return false;
        }
        return true;
        // XXXX More...
    }
    return false;
}

void TextInputPane::ProcessSizeChange() {
    BoxPane::ProcessSizeChange();

    // Update the character width and cursor position if active.
    if (is_active_) {
        UpdateCharWidth_();
        MoveCursor_(cursor_pos_);
    }
}

void TextInputPane::UpdateCharWidth_() {
    ASSERT(text_pane_);

    // This uses a monospace font, so each character should be the same width.
    char_width_ = text_pane_->GetTextSize()[0] / text_pane_->GetText().size();
    ASSERT(char_width_ > 0);

    // Also undo the effects of TextPane scaling on the cursor.
    auto cursor = SG::FindNodeUnderNode(*this, "Cursor");
    const auto text_scale = text_pane_->GetScale();
    cursor->SetScale(Vector3f(text_scale[0], 1, 1));
}

void TextInputPane::SetBackgroundColor_(const std::string &color_name) {
    auto bg = SG::FindNodeUnderNode(*this, "Background");
    bg->SetBaseColor(ColorManager::GetSpecialColor(color_name));
}

void TextInputPane::ShowCursor_(bool show) {
    auto cursor = SG::FindNodeUnderNode(*this, "Cursor");
    cursor->SetEnabled(SG::Node::Flag::kTraversal, show);
    MoveCursor_(cursor_pos_);
}

void TextInputPane::MoveCursor_(size_t new_pos) {
    auto cursor = SG::FindNodeUnderNode(*this, "Cursor");
    cursor_pos_ = new_pos;
    const float offset = .2f + GetPadding();  // Move just to left of character.
    const float x = -.5f + (new_pos + offset) * char_width_ / GetSize()[0];
    cursor->SetTranslation(Vector3f(x, 0, 0));
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
