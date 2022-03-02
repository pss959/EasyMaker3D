#include "Panes/TextInputPane.h"

#include <functional>
#include <unordered_map>
#include <vector>

#include "ClickInfo.h"
#include "Enums/TextAction.h"
#include "Event.h"
#include "Managers/ColorManager.h"
#include "Math/Linear.h"
#include "Panes/TextPane.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

// ----------------------------------------------------------------------------
// TextInputPane::StateStack_ class.
// ----------------------------------------------------------------------------

/// This class manages undo/redo for the TextInputPane::Impl_ class using a
/// stack of instances storing the current text, cursor position, and selection
/// state. There is always at least one instance on the stack to hold the
/// current state.
class TextInputPane::StateStack_ {
  public:
    StateStack_() { Clear(); }

    /// Clears to initial state.
    void Clear();

    /// Pushes a new entry with the current state values.
    void Push();

    void SetText(const std::string &text) { Top_().text          = text; }
    void SetCursorPos(size_t pos)         { Top_().cursor_pos    = pos;  }

    void SetSelection(size_t start, size_t end) {
        auto &top = Top_();
        top.sel_start_pos = start;
        top.sel_end_pos   = end;
    }
    void ClearSelection() { SetSelection(0, 0); }

    const std::string & GetText() const { return Top_().text;          }
    size_t GetCursorPos()         const { return Top_().cursor_pos;    }
    size_t GetSelectionStartPos() const { return Top_().sel_start_pos; }
    size_t GetSelectionEndPos()   const { return Top_().sel_end_pos;   }
    bool HasSelection() const {
        return GetSelectionStartPos() != GetSelectionEndPos();
    }

    bool Undo();  ///< Returns false if nothing to undo.
    bool Redo();  ///< Returns false if nothing to redo.

  private:
    /// Struct storing all necessary current state.
    struct State_ {
        std::string text;           ///< Displayed text
        size_t      cursor_pos;     ///< Character position of cursor.
        size_t      sel_start_pos;  ///< Selection start character position.
        size_t      sel_end_pos;    ///< Selection end character position.
    };

    /// Stack of State_ instances. This is stored as a vector because redo
    /// requires random access.
    std::vector<State_> stack_;

    /// Index to use for next stack entry.
    size_t index_;

    /// Returns the top of the stack.
    const State_ & Top_() const {
        ASSERT(index_ < stack_.size());
        return stack_[index_];
    }

    /// Returns the top of the stack.
    State_ & Top_() {
        ASSERT(index_ < stack_.size());
        return stack_[index_];
    }
};

void TextInputPane::StateStack_::Clear() {
    stack_.clear();

    // Create an instance to store the current values.
    State_ state;
    state.cursor_pos = state.sel_start_pos = state.sel_end_pos = 0;
    stack_.push_back(state);

    index_ = 0;
}

void TextInputPane::StateStack_::Push() {
    // Clear out undone state.
    if (index_ < stack_.size())
        stack_.erase(stack_.begin() + index_ + 1, stack_.end());

    stack_.push_back(Top_());
    ++index_;
}

bool TextInputPane::StateStack_::Undo() {
    if (index_ > 1U) {  // Never undo the first instance.
        --index_;
        return true;
    }
    return false;
}

bool TextInputPane::StateStack_::Redo() {
    if (index_ + 1 < stack_.size()) {
        ++index_;
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// TextInputPane::Impl_ class.
// ----------------------------------------------------------------------------

class TextInputPane::Impl_ {
  public:
    Impl_(ContainerPane &root_pane, const std::string &initial_text,
          float padding);

    void SetValidationFunc(const ValidationFunc &func) {
        validation_func_ = func;
    }
    void SetInitialText(const std::string &text);
    std::string GetText() const;
    void Activate();
    void Deactivate();
    bool HandleEvent(const Event &event);
    void SizeChanged(const Pane &initiating_pane);

  private:
    /// Maps key string sequences to actions.
    static std::unordered_map<std::string, TextAction> s_action_map_;

    /// Function to invoke to determine if the current text is valid.
    ValidationFunc validation_func_;

    /// Parts.
    ContainerPane &root_pane_;     ///< Root Pane.
    TextPanePtr    text_pane_;     ///< Displays the current text.
    SG::NodePtr    cursor_;        ///< Cursor.
    SG::NodePtr    selection_;     ///< Selection.
    SG::NodePtr    background_;    ///< Pane background.

    std::string    initial_text_;       ///< Initial text string.
    const float    padding_;            ///< Padding around text.
    bool           is_active_ = false;  ///< True while active (editing).

    /// Width of a single text character in monospace font.
    float char_width_ = 0;

    /// Class managing undo/redo state.
    StateStack_ stack_;

    static void InitActionMap_();

    void ProcessAction_(TextAction action);
    void InsertChars(const std::string &chars);
    void DeleteChars_(size_t start_pos, int count, int cursor_motion);
    void ChangeText_(const std::string &new_text, bool add_to_stack = true);
    void ChangeSelectionBy_(int amount);
    void ChangeSelectionTo_(int start, int end);  ///< -1 means current.
    void UpdateCharWidth_();
    void UpdateBackgroundColor_();
    void ShowCursorAndSelection_(bool show);
    void MoveCursorBy_(int amount);
    void MoveCursorTo_(size_t new_pos);
    void UpdateFromState_();
    void ProcessClick_(const ClickInfo &info);

    /// Converts a character position to a local X coordinate value.
    float CharPosToX_(size_t pos) const;

    /// Converts a local X coordinate to a character position.
    size_t XToCharPos_(float x) const;

    friend class Parser::Registry;
};

std::unordered_map<std::string, TextAction> TextInputPane::Impl_::s_action_map_;

TextInputPane::Impl_::Impl_(ContainerPane &root_pane,
                            const std::string &initial_text, float padding) :
    root_pane_(root_pane),
    initial_text_(initial_text),
    padding_(padding) {

    // Set up the static action map if not already done.
    if (s_action_map_.empty())
        InitActionMap_();

    // Find all the parts.
    text_pane_  = root_pane_.FindTypedPane<TextPane>("TextPane");
    cursor_     = SG::FindNodeUnderNode(root_pane_, "Cursor");
    selection_  = SG::FindNodeUnderNode(root_pane_, "Selection");
    background_ = SG::FindNodeUnderNode(root_pane_, "Background");
    auto button = SG::FindTypedNodeUnderNode<PushButtonWidget>(
        root_pane_, "Button");

    // Set up the TextPane.
    if (text_pane_->GetText() != initial_text_)
        ChangeText_(initial_text_);

    // Set up the button used to activate the Pane or move the cursor.
    button->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ ProcessClick_(info); });

    // Push one entry on the stack to hold the current state.
}

void TextInputPane::Impl_::SetInitialText(const std::string &text) {
    initial_text_ = text;
    ChangeText_(text);

    stack_.Clear();
}

std::string TextInputPane::Impl_::GetText() const {
    return text_pane_ ? text_pane_->GetText() : std::string();
}

void TextInputPane::Impl_::Activate() {
    if (! is_active_) {
        is_active_ = true;
        UpdateCharWidth_();
        UpdateBackgroundColor_();
        ShowCursorAndSelection_(true);
    }
}

void TextInputPane::Impl_::Deactivate() {
    if (is_active_) {
        is_active_ = false;
        UpdateBackgroundColor_();
        ShowCursorAndSelection_(false);
    }
}

bool TextInputPane::Impl_::HandleEvent(const Event &event) {
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

void TextInputPane::Impl_::SizeChanged(const Pane &initiating_pane) {
    // If the change came from our TextPane, do nothing, since changes to the
    // text will cause this to happen.
    if (&initiating_pane == text_pane_.get())
        return;

    // Update the character width and cursor position if active.
    if (is_active_) {
        UpdateCharWidth_();
        MoveCursorTo_(stack_.GetCursorPos());
    }
}

void TextInputPane::Impl_::InitActionMap_() {
    s_action_map_["<Ctrl>Backspace"] = TextAction::kClear;
    s_action_map_["<Ctrl>K"]         = TextAction::kDeleteToStart;
    s_action_map_["<Ctrl>Z"]         = TextAction::kRedo;
    s_action_map_["<Ctrl>a"]         = TextAction::kSelectAll;
    s_action_map_["<Ctrl>b"]         = TextAction::kMovePrevious;
    s_action_map_["<Ctrl>d"]         = TextAction::kDeleteNext;
    s_action_map_["<Ctrl>e"]         = TextAction::kMoveToEnd;
    s_action_map_["<Ctrl>f"]         = TextAction::kMoveNext;
    s_action_map_["<Ctrl>k"]         = TextAction::kDeleteToEnd;
    s_action_map_["<Ctrl>z"]         = TextAction::kUndo;
    s_action_map_["<Shift>Down"]     = TextAction::kSelectToEnd;
    s_action_map_["<Shift>Left"]     = TextAction::kSelectPrevious;
    s_action_map_["<Shift>Right"]    = TextAction::kSelectNext;
    s_action_map_["<Shift>Up"]       = TextAction::kSelectToStart;
    s_action_map_["Backspace"]       = TextAction::kDeletePrevious;
    s_action_map_["Down"]            = TextAction::kMoveToEnd;
    s_action_map_["Left"]            = TextAction::kMovePrevious;
    s_action_map_["Left"]            = TextAction::kMovePrevious;
    s_action_map_["Right"]           = TextAction::kMoveNext;
    s_action_map_["Up"]              = TextAction::kMoveToStart;
}

void TextInputPane::Impl_::ProcessAction_(TextAction action) {
    //
    // Some rules:
    //  - Changing the cursor position always clears the current selection.
    //  - Changing the selection can also change the cursor position. XXXX
    //  - Inserting or deleting text always clears the current selection; XXXX
    //     inserting replaces the current selection. XXXX
    //  - Undo/redo restores the text and selection.
    //

    // Current state.
    const size_t char_count         = text_pane_->GetText().size();
    const size_t cursor_pos         = stack_.GetCursorPos();
    const size_t sel_start_pos      = stack_.GetSelectionStartPos();
    const size_t sel_end_pos        = stack_.GetSelectionEndPos();

    // Boundary conditions.
    const bool is_cursor_at_start = cursor_pos == 0;
    const bool is_cursor_at_end   = cursor_pos == char_count;
    const bool is_sel_at_start =
        stack_.HasSelection() ? sel_start_pos == 0 : is_cursor_at_start;
    const bool is_sel_at_end =
        stack_.HasSelection() ? sel_end_pos == char_count : is_cursor_at_end;

    switch (action) {
      case TextAction::kClear:
        ChangeText_("");
        break;
      case TextAction::kDeleteNext:
        if (! is_cursor_at_end)
            DeleteChars_(cursor_pos, 1, 0);
        break;
      case TextAction::kDeletePrevious:
        if (! is_cursor_at_start)
            DeleteChars_(cursor_pos - 1, 1, -1);
        break;
      case TextAction::kDeleteToEnd:
        if (! is_cursor_at_end)
            DeleteChars_(cursor_pos, -1, 0);
        break;
      case TextAction::kDeleteToStart:
        if (! is_cursor_at_start)
            DeleteChars_(0, cursor_pos, -cursor_pos);
        break;
      case TextAction::kMoveNext:
        if (! is_cursor_at_end)
            MoveCursorBy_(1);
        break;
      case TextAction::kMovePrevious:
        if (! is_cursor_at_start)
            MoveCursorBy_(-1);
        break;
      case TextAction::kMoveToEnd:
        if (! is_cursor_at_end)
            MoveCursorTo_(char_count);
        break;
      case TextAction::kMoveToStart:
        if (! is_cursor_at_start)
            MoveCursorTo_(0);
        break;
      case TextAction::kRedo:
        if (stack_.Redo())
            UpdateFromState_();
        break;
      case TextAction::kSelectAll:
        if (! is_sel_at_start || ! is_sel_at_end)
            ChangeSelectionTo_(0, char_count);
        break;
      case TextAction::kSelectNext:
        if (! is_sel_at_end)
            ChangeSelectionBy_(1);
        break;
      case TextAction::kSelectNone:
        if (sel_start_pos != sel_end_pos)
            ChangeSelectionTo_(0, 0);
        break;
      case TextAction::kSelectPrevious:
        if (! is_sel_at_start)
            ChangeSelectionBy_(-1);
        break;
      case TextAction::kSelectToEnd:
        if (! is_sel_at_end)
            ChangeSelectionTo_(-1, char_count);
        break;
      case TextAction::kSelectToStart:
        if (! is_sel_at_start)
            ChangeSelectionTo_(0, -1);
        break;
      case TextAction::kUndo:
        if (stack_.Undo())
            UpdateFromState_();
        break;
    }
}

void TextInputPane::Impl_::InsertChars(const std::string &chars) {
    std::string text = text_pane_->GetText();
    text.insert(stack_.GetCursorPos(), chars);
    ChangeText_(text);
    MoveCursorBy_(chars.size());
}

void TextInputPane::Impl_::DeleteChars_(size_t start_pos, int count,
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
        MoveCursorBy_(cursor_motion);
}

void TextInputPane::Impl_::ChangeText_(const std::string &new_text,
                                       bool add_to_stack) {
    ASSERT(text_pane_);
    text_pane_->SetText(new_text);
    UpdateBackgroundColor_();

    // If the text is larger than the base size, notify.
    const Vector2f text_size = text_pane_->GetBaseSize();
    const Vector2f pane_size = root_pane_.GetBaseSize();
    if (text_size[0] > pane_size[0] || text_size[1] > pane_size[1])
        SizeChanged(*text_pane_);

    if (add_to_stack) {
        stack_.Push();
        stack_.SetText(new_text);
    }
}

void TextInputPane::Impl_::ChangeSelectionBy_(int amount) {
    const size_t cursor_pos = stack_.GetCursorPos();
    const size_t start      = stack_.GetSelectionStartPos();
    const size_t end        = stack_.GetSelectionEndPos();

    size_t new_start = start;
    size_t new_end   = end;

    bool move_cursor_to_start;

    if (stack_.HasSelection()) {
        ASSERT(cursor_pos == start || cursor_pos == end);

        // Modify selection at the end with the cursor.
        if (cursor_pos == start)
            new_start += amount;
        else
            new_end   += amount;
        move_cursor_to_start = cursor_pos == start;
    }
    else {
        // No current selection: select one character relative to cursor.
        new_start = new_end = cursor_pos;
        if (amount < 0)
            new_start += amount;
        else
            new_end   += amount;
        move_cursor_to_start = amount < 0;
    }

    // Update the selection and move the cursor to the changed position.
    ChangeSelectionTo_(new_start, new_end);
    MoveCursorTo_(move_cursor_to_start ? new_start : new_end);
}

void TextInputPane::Impl_::ChangeSelectionTo_(int start, int end) {
    if (start == end) {
        stack_.ClearSelection();
        selection_->SetEnabled(false);
    }
    else {
        const bool has_sel = stack_.HasSelection();

        // A negative start/end value means to use the current selection
        // start/end position or cursor position.
        const size_t start_pos =
            start >= 0 ? start :
            (has_sel ? stack_.GetSelectionStartPos() : stack_.GetCursorPos());
        const size_t end_pos =
            end >= 0 ? end :
            (has_sel ? stack_.GetSelectionEndPos() : stack_.GetCursorPos());

        ASSERT(start_pos < end_pos);
        stack_.SetSelection(start_pos, end_pos);

        selection_->SetEnabled(true);

        const float x0 = CharPosToX_(start_pos);
        const float x1 = CharPosToX_(end_pos);
        selection_->SetScale(Vector3f(x1 - x0, 1, 1));
        selection_->SetTranslation(Vector3f(.5f * (x0 + x1), 0, 0));
    }
}

void TextInputPane::Impl_::UpdateCharWidth_() {
    ASSERT(text_pane_);

    // This uses a monospace font, so each character should be the same width.
    char_width_ = text_pane_->GetTextSize()[0] / text_pane_->GetText().size();
    ASSERT(char_width_ > 0);

    // Also undo the effects of TextPane scaling on the cursor.
    const auto text_scale = text_pane_->GetScale();
    cursor_->SetScale(Vector3f(text_scale[0], 1, 1));
}

void TextInputPane::Impl_::UpdateBackgroundColor_() {
    // Have to wait for Ion to be set up first.
    if (root_pane_.GetIonNode()) {
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

        background_->SetBaseColor(ColorManager::GetSpecialColor(color_name));
    }
}

void TextInputPane::Impl_::ShowCursorAndSelection_(bool show) {
    cursor_->SetEnabled(show);
    selection_->SetEnabled(show);
    if (show)
        UpdateFromState_();
}

void TextInputPane::Impl_::MoveCursorBy_(int amount) {
    if (stack_.HasSelection()) {
        // If there is a current selection, move relative to one of the
        // endpoints.
        if (amount > 0)
            MoveCursorTo_(stack_.GetSelectionEndPos() + amount);
        else
            MoveCursorTo_(stack_.GetSelectionStartPos() + amount);
    }
    else {
        // Otherwise, move relative to the current cursor position.
        MoveCursorTo_(stack_.GetCursorPos() + amount);
    }

    // Changing the cursor position always clears the current selection.
    ChangeSelectionTo_(0, 0);
}

void TextInputPane::Impl_::MoveCursorTo_(size_t new_pos) {
    // Set the cursor position.
    const float x = CharPosToX_(new_pos);
    cursor_->SetTranslation(Vector3f(x, 0, 0));

    stack_.SetCursorPos(new_pos);

    // Clear the current selection.
}

void TextInputPane::Impl_::UpdateFromState_() {
    ChangeText_(stack_.GetText(), false);
    MoveCursorTo_(stack_.GetCursorPos());
    ChangeSelectionTo_(stack_.GetSelectionStartPos(),
                       stack_.GetSelectionEndPos());
}

void TextInputPane::Impl_::ProcessClick_(const ClickInfo &info) {
    // If the pane is already active, adjust the cursor position.
    if (is_active_) {
        MoveCursorTo_(XToCharPos_(info.hit.point[0]));
    }
    else {
        // Otherwise, just take focus and activate.
        root_pane_.TakeFocus();
        root_pane_.Activate();
    }
}

float TextInputPane::Impl_::CharPosToX_(size_t pos) const {
    // The X value ranges from -.5 to +.5 across the TextInputPane.  The text
    // starts just after the padding, so start there and add the appropriate
    // number of character widths.
    return -.5f + (padding_ + pos * char_width_) / root_pane_.GetSize()[0];
}

size_t TextInputPane::Impl_::XToCharPos_(float x) const {
    // The math here is the inverse of CharPosToX_().
    const float pane_width = root_pane_.GetSize()[0];
    const float pos = ((x + .5f) * pane_width - padding_) / char_width_;

    // Round and clamp to the number of characters in the text string.
    const size_t text_size = text_pane_->GetText().size();
    return static_cast<size_t>(Clamp(pos + .5f, 0, text_size));
}

// ----------------------------------------------------------------------------
// TextInputPane functions.
// ----------------------------------------------------------------------------

TextInputPane::TextInputPane() {
}

void TextInputPane::AddFields() {
    AddField(initial_text_);
    BoxPane::AddFields();
}

void TextInputPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate())
        impl_.reset(new Impl_(*this, initial_text_, GetPadding()));
}

void TextInputPane::SetValidationFunc(const ValidationFunc &func) {
    impl_->SetValidationFunc(func);
}

void TextInputPane::SetInitialText(const std::string &text) {
    impl_->SetInitialText(text);
}

std::string TextInputPane::GetText() const {
    return impl_->GetText();
}

void TextInputPane::Activate() {
    impl_->Activate();
}

void TextInputPane::Deactivate() {
    impl_->Deactivate();
}

bool TextInputPane::HandleEvent(const Event &event) {
    return impl_->HandleEvent(event);
}

void TextInputPane::SizeChanged(const Pane &initiating_pane) {
    BoxPane::SizeChanged(initiating_pane);
    if (impl_)
        impl_->SizeChanged(initiating_pane);
}
