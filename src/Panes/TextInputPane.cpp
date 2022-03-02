#include "Panes/TextInputPane.h"

#include <functional>

#include "ClickInfo.h"
#include "Event.h"
#include "Managers/ColorManager.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

std::unordered_map<std::string, TextAction> TextInputPane::s_action_map_;

void TextInputPane::AddFields() {
    AddField(initial_text_);
    BoxPane::AddFields();
}

void TextInputPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate()) {
        // Access and set up the TextPane.
        text_pane_ = FindTypedPane<TextPane>("TextPane");
        if (text_pane_->GetText() != initial_text_.GetValue())
            ChangeText_(initial_text_);

        // Set up the PushButtonWidget.
        auto button =
            SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
        button->GetClicked().AddObserver(
            this, [&](const ClickInfo &info){ ProcessClick_(info); });
    }

    // Set up the action map if not already done.
    if (s_action_map_.empty())
        InitActionMap_();
}

void TextInputPane::SetInitialText(const std::string &text) {
    initial_text_ = text;
    ChangeText_(text);

    text_stack_.clear();
}

std::string TextInputPane::GetText() const {
    return text_pane_ ? text_pane_->GetText() : std::string();
}

void TextInputPane::Activate() {
    if (! is_active_) {
        is_active_ = true;
        UpdateCharWidth_();
        UpdateBackgroundColor_();
        ShowCursor_(true);
    }
}

void TextInputPane::Deactivate() {
    if (is_active_) {
        is_active_ = false;
        UpdateBackgroundColor_();
        ShowCursor_(false);
    }
}

bool TextInputPane::HandleEvent(const Event &event) {
    bool ret = false;

    if (is_active_) {
        if (event.flags.Has(Event::Flag::kKeyPress)) {
            // Check first for a key sequence in the action map.
            const std::string key_string = event.GetKeyString();
            const auto it = s_action_map_.find(key_string);
            if (it != s_action_map_.end()) {
                ProcessAction_(it->second);
                ret = true;
            }

            // Otherwise, insert the text.
            else if (! event.modifiers.HasAny() &&
                     event.key_name.size() == 1U) {
                InsertChars(event.key_name);
                ret = true;
            }
        }
    }
    return ret;
}

void TextInputPane::SizeChanged(const Pane &initiating_pane) {
    // If the change came from our TextPane, do nothing, since changes to the
    // text will cause this to happen.
    if (&initiating_pane == text_pane_.get())
        return;

    BoxPane::SizeChanged(initiating_pane);

    // Update the character width and cursor position if active.
    if (is_active_) {
        UpdateCharWidth_();
        MoveCursor_(cursor_pos_);
    }
}

void TextInputPane::InitActionMap_() {
    s_action_map_["<Ctrl>Backspace"] = TextAction::kClear;
    s_action_map_["<Ctrl>K"]         = TextAction::kDeleteToStart;
    s_action_map_["<Ctrl>Z"]         = TextAction::kRedo;
    s_action_map_["<Ctrl>a"]         = TextAction::kSelectAll;
    s_action_map_["<Ctrl>d"]         = TextAction::kDeleteNext;
    s_action_map_["<Ctrl>e"]         = TextAction::kMoveToEnd;
    s_action_map_["<Ctrl>k"]         = TextAction::kDeleteToEnd;
    s_action_map_["<Ctrl>z"]         = TextAction::kUndo;
    s_action_map_["Backspace"]       = TextAction::kDeletePrevious;
    s_action_map_["Down"]            = TextAction::kMoveToEnd;
    s_action_map_["Left"]            = TextAction::kMovePrevious;
    s_action_map_["Right"]           = TextAction::kMoveNext;
    s_action_map_["Up"]              = TextAction::kMoveToStart;
}

void TextInputPane::ProcessAction_(TextAction action) {
    auto is_at_start = [&](){ return cursor_pos_ == 0; };
    auto is_at_end   = [&](){
        return cursor_pos_ == text_pane_->GetText().size();
    };

    switch (action) {
      case TextAction::kClear:
        ChangeText_("");
        break;
      case TextAction::kDeleteNext:
        if (! is_at_end())
            DeleteChars_(cursor_pos_, 1, 0);
        break;
      case TextAction::kDeletePrevious:
        if (! is_at_start())
            DeleteChars_(cursor_pos_ - 1, 1, -1);
        break;
      case TextAction::kDeleteToEnd:
        if (! is_at_end())
            DeleteChars_(cursor_pos_, -1, 0);
        break;
      case TextAction::kDeleteToStart:
        if (! is_at_start())
            DeleteChars_(0, cursor_pos_, -cursor_pos_);
        break;
      case TextAction::kMoveNext:
        if (! is_at_end())
            MoveCursor_(cursor_pos_ + 1);
        break;
      case TextAction::kMovePrevious:
        if (! is_at_start())
            MoveCursor_(cursor_pos_ - 1);
        break;
      case TextAction::kMoveToEnd:
        if (! is_at_end())
            MoveCursor_(text_pane_->GetText().size());
        break;
      case TextAction::kMoveToStart:
        if (! is_at_start())
            MoveCursor_(0);
        break;
      case TextAction::kRedo:
        if (stack_index_ < text_stack_.size())
            ChangeText_(text_stack_[stack_index_++], false);
        break;
      case TextAction::kSelectAll:
        // XXXX
        break;
      case TextAction::kSelectNone:
        // XXXX
        break;
      case TextAction::kUndo:
        if (stack_index_ > 0)
            ChangeText_(text_stack_[--stack_index_], false);
        break;
    }
}

void TextInputPane::InsertChars(const std::string &chars) {
    std::string text = text_pane_->GetText();
    text.insert(cursor_pos_, chars);
    ChangeText_(text);
    MoveCursor_(cursor_pos_ + chars.size());
}

void TextInputPane::DeleteChars_(size_t start_pos, int count,
                                 int cursor_motion) {
    std::string text = text_pane_->GetText();
    ASSERT(start_pos < text.size());
    // A negative count means delete to end.
    if (count < 0)
        text.erase(start_pos, std::string::npos);
    else
        text.erase(start_pos, static_cast<size_t>(count));
    ChangeText_(text);
    if (cursor_motion)
        MoveCursor_(cursor_pos_ + cursor_motion);
}

void TextInputPane::ChangeText_(const std::string &new_text,
                                bool add_to_stack) {
    ASSERT(text_pane_);
    text_pane_->SetText(new_text);
    UpdateBackgroundColor_();

    // If the text is larger than the base size, notify.
    const Vector2f text_size = text_pane_->GetBaseSize();
    const Vector2f this_size = GetBaseSize();
    if (text_size[0] > this_size[0] || text_size[1] > this_size[1])
        SizeChanged(*text_pane_);

    if (add_to_stack) {
        if (stack_index_ < text_stack_.size())
            text_stack_.erase(text_stack_.begin() + stack_index_ + 1,
                              text_stack_.end());
        text_stack_.push_back(new_text);
        stack_index_ = text_stack_.size() - 1;
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

void TextInputPane::UpdateBackgroundColor_() {
    // Have to wait for Ion to be set up first.
    if (GetIonNode()) {
        std::string color_name;
        if (is_active_) {
            const std::string text = text_pane_->GetText();
            const bool is_valid = ! validation_func_ || validation_func_(text);
            color_name = is_valid ?
                "TextInputActiveColor" : "TextInputErrorColor";
        }
        else {
            color_name = "TextInputInactiveColor";
        }

        auto bg = SG::FindNodeUnderNode(*this, "Background");
        bg->SetBaseColor(ColorManager::GetSpecialColor(color_name));
    }
}

void TextInputPane::ShowCursor_(bool show) {
    auto cursor = SG::FindNodeUnderNode(*this, "Cursor");
    cursor->SetEnabled(show);
    MoveCursor_(cursor_pos_);
}

void TextInputPane::MoveCursor_(size_t new_pos) {
    // The X value ranges from -.5 to +.5 across the TextInputPane.  The text
    // starts just after the padding, so start there and add the appropriate
    // number of character widths.
    const float pane_width = GetSize()[0];
    const float x = -.5f + (GetPadding() + new_pos * char_width_) / pane_width;

    // Set the cursor position.
    auto cursor = SG::FindNodeUnderNode(*this, "Cursor");
    cursor->SetTranslation(Vector3f(x, 0, 0));

    cursor_pos_ = new_pos;
}

void TextInputPane::ProcessClick_(const ClickInfo &info) {
    // If the pane is already active, adjust the cursor position.
    if (is_active_) {
        // The math here is the inverse of MoveCursor_() so that the new
        // position results in the same approximate X location.
        const size_t text_size = text_pane_->GetText().size();
        const float pane_width = GetSize()[0];
        const float x = info.hit.point[0];
        const float pos = ((x + .5f) * pane_width - GetPadding()) / char_width_;
        MoveCursor_(static_cast<size_t>(Clamp(pos + .5f, 0, text_size)));
    }
    else {
        // Otherwise, just take focus and activate.
        TakeFocus();
        Activate();
    }
}
