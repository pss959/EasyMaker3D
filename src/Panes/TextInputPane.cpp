#include "Panes/TextInputPane.h"

#include <algorithm>
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
// TextInputPane::Range_ struct.
// ----------------------------------------------------------------------------

/// The TextInputPane::Range_ struct defines a range of character positions.
struct TextInputPane::Range_ {
    size_t start = 0;
    size_t end   = 0;

    /// Default constructor creates an empty range.
    Range_() {}

    /// Constructor taking start and end values.
    Range_(size_t s, size_t e) : start(s), end(e) {
        ASSERT((s == 0 && e == 0) || s < e);
    }

    bool IsEmpty() const { return start == end; }
};

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

    void SetSelectionRange(const Range_ range) { Top_().sel_range = range; }
    void ClearSelection() { SetSelectionRange(Range_()); }

    const std::string & GetText()      const { return Top_().text;          }
    size_t              GetCursorPos() const { return Top_().cursor_pos; }
    const Range_ & GetSelectionRange() const { return Top_().sel_range; }

    /// Returns the new position of the cursor if moved by the given signed
    /// amount, if possible.
    size_t GetMovedCursorBy(int amount) const;

    /// Returns the count of characters in the current text.
    size_t GetCharCount() const { return GetText().size(); }

    /// Returns the full range for all current characters.
    Range_ GetFullRange() const { return Range_(0, GetCharCount()); }

    /// Creates a Range_ that is contains the given number of characters to the
    /// right of the current cursor. A count of 0 means all characters to the
    /// end of the text.
    Range_ GetRangeRightOfCursor(size_t count) const;

    /// Creates a Range_ that is contains the given number of characters to the
    /// left of the current cursor. A count of 0 means all characters to the
    /// beginning of the text.
    Range_ GetRangeLeftOfCursor(size_t count) const;

    /// Modifies the Range_ representing the current selection by moving the
    /// appropriate end in the given direction by the given number of
    /// characters. If the count is 0, this moves as far as possible in that
    /// direction.
    Range_ GetModifiedSelectionRange(bool to_left, size_t count) const;

    bool HasSelection() const { return ! GetSelectionRange().IsEmpty(); }

    /// Returns the position relative to the current cursor position in the
    /// given direction by the given count. A count of 0 means move as far as
    /// possible. Returns the new position.
    size_t GetCursorRelativePosition(bool is_left, size_t count);

    /// Returns the position created by moving left the given number of
    /// characters from the given position, clamping if necessary.
    size_t MoveLeft(size_t pos, size_t count) const {
        return count == 0 || pos <= count ? 0 : pos - count;
    }
    /// Returns the position created by moving right the given number of
    /// characters from the given position, clamping if necessary.
    size_t MoveRight(size_t pos, size_t count) const {
        return std::min(pos + count, GetCharCount());
    }

    bool Undo();  ///< Returns false if nothing to undo.
    bool Redo();  ///< Returns false if nothing to redo.

  private:
    /// Struct storing all necessary current state.
    struct State_ {
        std::string text;        ///< Displayed text
        size_t      cursor_pos;  ///< Character position of cursor.
        Range_      sel_range;   ///< Selection start/end character positions.
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
    state.cursor_pos = 0;
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

TextInputPane::Range_
TextInputPane::StateStack_::GetRangeRightOfCursor(size_t count) const {
    Range_ range;
    range.start = GetCursorPos();
    range.end   = MoveRight(range.start, count);
    return range;
}

TextInputPane::Range_
TextInputPane::StateStack_::GetRangeLeftOfCursor(size_t count) const {
    Range_ range;
    range.end   = GetCursorPos();
    range.start = MoveLeft(range.end, count);
    return range;
}

TextInputPane::Range_
TextInputPane::StateStack_::GetModifiedSelectionRange(bool to_left,
                                                      size_t count) const {
    const Range_ sel_range = GetSelectionRange();
    Range_       new_range = sel_range;

    // If there is no selection, modify the range at the end with the cursor.
    const size_t cursor_pos = GetCursorPos();
    if (sel_range.IsEmpty()) {
        ASSERT(cursor_pos == sel_range.start || cursor_pos == sel_range.end);
        if (cursor_pos == sel_range.start) {
            new_range.start = to_left ? MoveLeft(new_range.start, count) :
                MoveRight(new_range.start, count);
        }
        else {
            new_range.end = to_left ? MoveLeft(new_range.end, count) :
                MoveRight(new_range.end, count);
        }
    }
    // If there is no current selection: select relative to the cursor.
    else {
        new_range.start = new_range.end = cursor_pos;
        if (to_left)
            new_range.start = MoveLeft(new_range.start, count);
        else
            new_range.end   = MoveRight(new_range.end, count);
    }
    return new_range;
}

size_t TextInputPane::StateStack_::GetCursorRelativePosition(bool is_left,
                                                             size_t count) {
    return is_left ?
        MoveLeft(GetCursorPos(), count) :
        MoveRight(GetCursorPos(), count);
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

    /// Initializes the displayed text to the given string.
    void InitText_(const std::string &text);

    /// Returns the Range_ to use to implement the given action.
    Range_ GetRangeForAction_(TextAction action) const;

    /// Deletes the characters in the given Range_. This moves the cursor to
    /// the deleted spot.
    void DeleteRange_(const Range_ &range);

    /// Inserts the given characters at the current cursor position. The cursor
    /// is moved to after the last inserted character.
    void InsertAtCursor_(const std::string &chars);

    /// Modifies the displayed text to be the given string, adding an entry to
    /// the stack so the change can be undone.
    void ModifyText_(const std::string &new_text);

    /// Changes the selection to encompass the given Range_.
    void ChangeSelection_(const Range_ &range);

    /// Returns the count of characters in the current text.
    size_t GetCharCount() const { return text_pane_->GetText().size(); }


    void UpdateCharWidth_();
    void UpdateBackgroundColor_();
    void ShowCursorAndSelection_(bool show);
    void MoveCursorBy_(bool is_left, size_t count);
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
    if (text_pane_->GetText() != initial_text_) {
        InitText_(initial_text_);
        ModifyText_(initial_text_);
    }

    // Set up the button used to activate the Pane or move the cursor.
    button->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ ProcessClick_(info); });

    // Push one entry on the stack to hold the current state.
}

void TextInputPane::Impl_::SetInitialText(const std::string &text) {
    initial_text_ = text;
    ModifyText_(text);
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
                InsertAtCursor_(event.key_name);
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
    s_action_map_["<Ctrl>Backspace"] = TextAction::kDeleteAll;
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

    switch (action) {
      case TextAction::kDeleteAll:
      case TextAction::kDeleteNext:
      case TextAction::kDeletePrevious:
      case TextAction::kDeleteSelected:
      case TextAction::kDeleteToEnd:
      case TextAction::kDeleteToStart:
        DeleteRange_(GetRangeForAction_(action));
        break;

      case TextAction::kMoveNext:
        MoveCursorBy_(false, 1);
        break;
      case TextAction::kMovePrevious:
        MoveCursorBy_(true, 1);
        break;
      case TextAction::kMoveToEnd:
        MoveCursorBy_(false, 0);
        break;
      case TextAction::kMoveToStart:
        MoveCursorBy_(true, 0);
        break;

      case TextAction::kSelectAll:
      case TextAction::kSelectNext:
      case TextAction::kSelectNone:
      case TextAction::kSelectPrevious:
      case TextAction::kSelectToEnd:
      case TextAction::kSelectToStart:
        ChangeSelection_(GetRangeForAction_(action));
        break;

      case TextAction::kUndo:
        if (stack_.Undo())
            UpdateFromState_();
        break;
      case TextAction::kRedo:
        if (stack_.Redo())
            UpdateFromState_();
        break;
    }
}

TextInputPane::Range_
TextInputPane::Impl_::GetRangeForAction_(TextAction action) const {
    Range_ range;

    switch (action) {
      case TextAction::kDeleteAll:
        range = stack_.GetFullRange();
        break;
      case TextAction::kDeleteNext:
        range = stack_.GetRangeRightOfCursor(1);
        break;
      case TextAction::kDeletePrevious:
        range = stack_.GetRangeLeftOfCursor(1);
        break;
      case TextAction::kDeleteSelected:
        range = stack_.HasSelection() ?
            stack_.GetSelectionRange() : stack_.GetRangeLeftOfCursor(1);
        break;
      case TextAction::kDeleteToEnd:
        range = stack_.GetRangeRightOfCursor(0);
        break;
      case TextAction::kDeleteToStart:
        range = stack_.GetRangeLeftOfCursor(0);
        break;

      case TextAction::kSelectAll:
        range = stack_.GetFullRange();
        break;
      case TextAction::kSelectNext:
        range = stack_.GetModifiedSelectionRange(false, 1);
        break;
      case TextAction::kSelectNone:
        break;  // Leave range empty.
      case TextAction::kSelectPrevious:
        range = stack_.GetModifiedSelectionRange(true, 1);
        break;
      case TextAction::kSelectToEnd:
        range = stack_.GetModifiedSelectionRange(false, 0);
        break;
      case TextAction::kSelectToStart:
        range = stack_.GetModifiedSelectionRange(true, 0);
        break;

      default:
        ASSERTM(false, "Invalid action: " + Util::EnumName(action));
    }

    return range;
}

void TextInputPane::Impl_::DeleteRange_(const Range_ &range) {
    // Ignore empty ranges.
    if (! range.IsEmpty()) {
        std::string text = stack_.GetText();
        ASSERT(range.end <= text.size());
        text.erase(text.begin() + range.start, text.end() + range.end);

        ModifyText_(text);
        MoveCursorTo_(range.start);
    }
}

void TextInputPane::Impl_::InsertAtCursor_(const std::string &chars) {
    std::string text = text_pane_->GetText();
    const size_t cursor_pos = stack_.GetCursorPos();
    ASSERT(cursor_pos <= text.size());
    text.insert(cursor_pos, chars);
    ModifyText_(text);
    MoveCursorTo_(cursor_pos + chars.size());
}

void TextInputPane::Impl_::InitText_(const std::string &text) {
    ASSERT(text_pane_);
    text_pane_->SetText(text);
    UpdateBackgroundColor_();

    // If the text is larger than the base size, notify.
    const Vector2f text_size = text_pane_->GetBaseSize();
    const Vector2f pane_size = root_pane_.GetBaseSize();
    if (text_size[0] > pane_size[0] || text_size[1] > pane_size[1])
        SizeChanged(*text_pane_);
}

void TextInputPane::Impl_::ModifyText_(const std::string &new_text) {
    InitText_(new_text);

    stack_.Push();
    stack_.SetText(new_text);
}

void TextInputPane::Impl_::ChangeSelection_(const Range_ &range) {
    if (range.IsEmpty()) {
        stack_.ClearSelection();
        selection_->SetEnabled(false);
        MoveCursorTo_(0);
    }
    else {
        const Range_ old_range = stack_.GetSelectionRange();

        ASSERT(range.start < range.end);
        stack_.SetSelectionRange(range);

        selection_->SetEnabled(true);

        const float x0 = CharPosToX_(range.start);
        const float x1 = CharPosToX_(range.end);
        selection_->SetScale(Vector3f(x1 - x0, 1, 1));
        selection_->SetTranslation(Vector3f(.5f * (x0 + x1), 0, 0));

        MoveCursorTo_(range.start != old_range.start ? range.start : range.end);
    }
}

void TextInputPane::Impl_::UpdateCharWidth_() {
    ASSERT(text_pane_);

    // This uses a monospace font, so each character should be the same width.
    char_width_ = text_pane_->GetTextSize()[0] / GetCharCount();
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

void TextInputPane::Impl_::MoveCursorBy_(bool is_left, size_t count) {
    // If there is a current selection, move relative to one of the endpoints.
    // Otherwise, move relative to the current cursor position.
    const Range_ range = stack_.GetSelectionRange();
    if (! range.IsEmpty())
        stack_.SetCursorPos(is_left ? range.start : range.end);
    MoveCursorTo_(stack_.GetCursorRelativePosition(is_left, count));

    // Changing the cursor position always clears the current selection.
    ChangeSelection_(Range_());
}

void TextInputPane::Impl_::MoveCursorTo_(size_t new_pos) {
    stack_.SetCursorPos(new_pos);

    // Set the cursor position.
    const float x = CharPosToX_(new_pos);
    cursor_->SetTranslation(Vector3f(x, 0, 0));
}

void TextInputPane::Impl_::UpdateFromState_() {
    InitText_(stack_.GetText());
    MoveCursorTo_(stack_.GetCursorPos());
    ChangeSelection_(stack_.GetSelectionRange());
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
    return static_cast<size_t>(Clamp(pos + .5f, 0, GetCharCount()));
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
