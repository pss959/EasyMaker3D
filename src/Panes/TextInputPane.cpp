#include "Panes/TextInputPane.h"

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>

#include "Base/Event.h"
#include "Base/VirtualKeyboard.h"
#include "Enums/TextAction.h"
#include "Math/Linear.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/TextPane.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/ColorMap.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "Widgets/GenericWidget.h"

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
// TextInputPane::State_ class.
// ----------------------------------------------------------------------------

/// This class manages the current text, cursor position, and selection state.
class TextInputPane::State_ {
  public:
    /// Sets the text string.
    void SetText(const Str &text) { text_ = text; }

    /// Returns the current text string.
    const Str & GetText() const { return text_; }

    /// Returns the count of characters in the text.
    size_t GetCharCount() const { return GetText().size(); }

    /// Sets the cursor position within the text.
    void SetCursorPos(size_t pos) {
        ASSERT(pos <= text_.size());
        cursor_pos_ = pos;
    }

    /// Returns the current cursor position within the text.
    size_t GetCursorPos() const { return cursor_pos_; }

    /// Sets the current selection range.
    void SetSelectionRange(const Range_ range) { sel_range_ = range; }

    /// Returns the current selection range.
    const Range_ & GetSelectionRange() const { return sel_range_; }

    /// Returns true if there is a non-empty selection range.
    bool HasSelection() const { return ! GetSelectionRange().IsEmpty(); }

    /// Returns the new position of the cursor if it were to be moved by the
    /// given signed amount, clamping if necessary.
    size_t GetMovedCursorBy(int amount) const;

    /// Returns the full range for all current characters.
    Range_ GetFullRange() const { return Range_(0, GetCharCount()); }

    /// Creates a Range_ that is contains the given number of characters to the
    /// right of the current cursor. A count of 0 means all characters to the
    /// end of the text.
    Range_ GetRangeRightOfCursor(size_t count) const {
        return Range_(cursor_pos_, MoveRight(cursor_pos_, count));
    }

    /// Creates a Range_ that is contains the given number of characters to the
    /// left of the current cursor. A count of 0 means all characters to the
    /// beginning of the text.
    Range_ GetRangeLeftOfCursor(size_t count) const {
        return Range_(MoveLeft(cursor_pos_, count), cursor_pos_);
    }

    /// Returns a Range_ representing the current selection after modifying it
    /// by moving the appropriate end in the given direction by the given
    /// number of characters. If the count is 0, this moves as far as possible
    /// in that direction.
    Range_ GetModifiedSelectionRange(bool to_left, size_t count) const;

    /// Returns the position relative to the current cursor position in the
    /// given direction by the given count. A count of 0 means move as far as
    /// possible. Returns the new position.
    size_t GetCursorRelativePosition(bool is_left, size_t count) {
        return is_left ? MoveLeft(cursor_pos_, count) :
            MoveRight(cursor_pos_, count);
    }

    /// Returns the position created by moving left the given number of
    /// characters from the given position, clamping if necessary.
    size_t MoveLeft(size_t pos, size_t count) const {
        return count == 0 || pos <= count ? 0 : pos - count;
    }

    /// Returns the position created by moving right the given number of
    /// characters from the given position, clamping if necessary.
    size_t MoveRight(size_t pos, size_t count) const {
        return count == 0 ? GetCharCount() :
            std::min(pos + count, GetCharCount());
    }

    void Validate() const;

    Str ToString() const;

  private:
    Str    text_;            ///< Displayed text
    size_t cursor_pos_ = 0;  ///< Character position of cursor.
    Range_ sel_range_;       ///< Selection start/end character positions.
};

TextInputPane::Range_
TextInputPane::State_::GetModifiedSelectionRange(bool to_left,
                                                 size_t count) const {
    Range_ new_range = sel_range_;

    // If there is no current selection: select relative to the cursor.
    if (sel_range_.IsEmpty()) {
        new_range.start = new_range.end = cursor_pos_;
        if (to_left)
            new_range.start = MoveLeft(cursor_pos_, count);
        else
            new_range.end   = MoveRight(cursor_pos_, count);
    }

    // If there a selection, modify the range at the end with the cursor,
    // except for count of 0, which always goes from the end in that direction.
    else {
        ASSERT(cursor_pos_ == sel_range_.start ||
               cursor_pos_ == sel_range_.end);
        const bool move_start = count == 0 ? to_left :
            cursor_pos_ == sel_range_.start;
        if (move_start)
            new_range.start = to_left ? MoveLeft(new_range.start, count) :
                MoveRight(new_range.start, count);
        else
            new_range.end = to_left ? MoveLeft(new_range.end, count) :
                MoveRight(new_range.end, count);
    }
    return new_range;
}

void TextInputPane::State_::Validate() const {
#if ENABLE_DEBUG_FEATURES
    ASSERT(cursor_pos_ <= GetCharCount());

    // The selection range must either be empty (start/end 0) or in order.
    ASSERT((sel_range_.start == 0 && sel_range_.end == 0) ||
           sel_range_.start < sel_range_.end);
    ASSERT(sel_range_.end <= GetCharCount());

    // If there is a selection, the cursor has to be at one end of it.
    ASSERT(sel_range_.IsEmpty() ||
           (cursor_pos_ == sel_range_.start || cursor_pos_ == sel_range_.end));
#endif
}

// LCOV_EXCL_START [debug only]
Str TextInputPane::State_::ToString() const {
    return "POS " + Util::ToString(cursor_pos_) +
        " SEL(" + Util::ToString(sel_range_.start) + "," +
        Util::ToString(sel_range_.end) + ") '" + text_ + "'";
}
// LCOV_EXCL_STOP

// ----------------------------------------------------------------------------
// TextInputPane::Stack_ class.
// ----------------------------------------------------------------------------

/// This class manages undo/redo for the TextInputPane::Impl_ class using a
/// stack of State_ instances. There is always at least one instance on the
/// stack to hold the current state.
class TextInputPane::Stack_ {
  public:
    Stack_() { Init(); }

    /// Sets the stack to its initial state.
    void Init();

    /// Pushes a new entry with the current state values.
    void Push();

    /// Returns the top of the stack.
    const State_ & GetTop() const {
        ASSERT(index_ < stack_.size());
        return stack_[index_];
    }

    /// Returns the top of the stack.
    State_ & GetTop() {
        ASSERT(index_ < stack_.size());
        return stack_[index_];
    }

    bool Undo();  ///< Returns false if nothing to undo.
    bool Redo();  ///< Returns false if nothing to redo.

    void Validate() const;

    void Dump() const;

  private:
    /// Stack of State_ instances. This is stored as a vector because redo
    /// requires random access.
    std::vector<State_> stack_;

    /// Index to use for next stack entry.
    size_t index_;
};

void TextInputPane::Stack_::Init() {
    // Clear the stack and create a default instance to store the initial
    // values.
    stack_.clear();
    stack_.push_back(State_());
    index_ = 0;
}

void TextInputPane::Stack_::Push() {
    // Clear out undone state.
    if (index_ < stack_.size())
        stack_.erase(stack_.begin() + index_ + 1, stack_.end());

    stack_.push_back(GetTop());
    ++index_;
}

bool TextInputPane::Stack_::Undo() {
    if (index_ > 0U) {  // Never undo the first instance.
        --index_;
        return true;
    }
    return false;
}

bool TextInputPane::Stack_::Redo() {
    if (index_ + 1 < stack_.size()) {
        ++index_;
        return true;
    }
    return false;
}

void TextInputPane::Stack_::Validate() const {
#if ENABLE_DEBUG_FEATURES
    ASSERT(! stack_.empty());
    for (const auto &state: stack_)
        state.Validate();
#endif
}

// LCOV_EXCL_START [debug only]
void TextInputPane::Stack_::Dump() const {
#if ENABLE_DEBUG_FEATURES
    ASSERT(! stack_.empty());
    std::cerr << "--- Stack_ with size " << stack_.size()
              << " and index " << index_ << ":\n";
    for (size_t i = 0; i < stack_.size(); ++i)
        std::cerr << "---  [" << i << "] " << stack_[i].ToString() << "\n";
#endif
}
// LCOV_EXCL_STOP

// ----------------------------------------------------------------------------
// TextInputPane::Impl_ class.
// ----------------------------------------------------------------------------

class TextInputPane::Impl_ : public IPaneInteractor {
  public:
    Impl_(ContainerPane &root_pane, float padding);

    void SetValidationFunc(const ValidationFunc &func) {
        validation_func_ = func;
    }
    void SetInitialText(const Str &text);
    Str  GetText() const;
    bool IsTextValid() const { return is_valid_; }

    // IPaneInteractor interface.
    virtual void SetVirtualKeyboard(const VirtualKeyboardPtr &vk) override {
        virtual_keyboard_ = vk;
    }
    virtual ClickableWidgetPtr GetActivationWidget() const override {
        return SG::FindTypedNodeUnderNode<GenericWidget>(root_pane_, "Widget");
    }
    BorderPtr GetFocusBorder() const override { return root_pane_.GetBorder(); }
    virtual void Activate() override;
    virtual void Deactivate() override;
    virtual bool IsActive() const override { return is_active_; }
    virtual bool HandleEvent(const Event &event) override;

    /// This is called when it is known that the pane's width has changed.
    void UpdateForWidthChange() {
        UpdateCharWidth_();
        MoveCursorTo_(GetState_().GetCursorPos());
    }

  private:
    /// Maps key string sequences to actions.
    static std::unordered_map<Str, TextAction> s_action_map_;

    /// Saves the VirtualKeyboard.
    VirtualKeyboardPtr virtual_keyboard_;

    /// Function to invoke to determine if the current text is valid.
    ValidationFunc validation_func_;

    /// Parts.
    ContainerPane &root_pane_;     ///< Root Pane.
    TextPanePtr    text_pane_;     ///< Displays the current text.
    SG::NodePtr    cursor_;        ///< Cursor.
    SG::NodePtr    selection_;     ///< Selection.
    SG::NodePtr    background_;    ///< Pane background.

    Str            initial_text_;        ///< Initial text string.
    const float    padding_;             ///< Padding around text.
    bool           is_valid_  = true;    ///< Whether current text is valid.
    bool           is_active_ = false;   ///< True while active (editing).
    size_t         drag_sel_start_ = 0;  ///< Start position of drag selection.

    /// Width of a single text character in monospace font.
    float          char_width_ = 0;

    /// Class managing undo/redo state.
    Stack_         stack_;

    static void InitActionMap_();

    void AttachToVirtualKeyboard_();
    void DetachFromVirtualKeyboard_();

    /// Returns the current State_().
    const State_ & GetState_() const { return stack_.GetTop(); }
    /// Returns the current State_().
    State_ & GetState_() { return stack_.GetTop(); }

    /// Sets the displayed text to the given string.
    void SetText_(const Str &text);

    /// Sets the displayed text to be the given string and pushes an entry on
    /// the stack so the change can be undone.
    void PushText_(const Str &new_text);

    void Finish_(bool is_accept);

    void ProcessAction_(TextAction action);

    /// Returns the Range_ to use to implement the given action.
    Range_ GetRangeForAction_(TextAction action) const;

    /// If the given range is not empty, this deletes the characters in the
    /// range and moves the cursor to the deleted spot.
    void DeleteRange_(const Range_ &range);

    /// Inserts the given characters to replace the current selection if there
    /// is any or at the current cursor position otherwise. The cursor is moved
    /// to after the last inserted character.
    void InsertChars_(const Str &chars);

    /// Changes the selection to encompass the given Range_ and updates the
    /// cursor based on the change.
    void ChangeSelection_(const Range_ &range);

    /// Changes the selection to encompass the given Range_. Does not modify
    /// the cursor.
    void SetSelectionRange_(const Range_ &range);

    /// Clears the current selection, leaving the cursor alone.
    void ClearSelection_() { SetSelectionRange_(Range_()); }

    /// Moves the cursor by the given amount in the given direction relative to
    /// the current selection (if there is any) or cursor. If count is 0, moves
    /// as far as possible in that direction.
    void MoveCursorBy_(bool is_left, size_t count);

    /// Moves the cursor to the given absolute position.
    void MoveCursorTo_(size_t new_pos);

    /// Updates the TextInputPane to match the current State_ and is_active_
    /// flag.
    void UpdateFromState_();

    void UpdateCharWidth_();
    void UpdateBackgroundColor_();
    void ShowCursorAndSelection_(bool show);
    void ProcessClick_(const ClickInfo &info);
    void ProcessDrag_(const DragInfo *info, bool is_start);

    /// Converts a character position to a local X coordinate value.
    float CharPosToX_(size_t pos) const;

    /// Converts a local X coordinate to a character position.
    size_t XToCharPos_(float x) const;

    friend class Parser::Registry;
};

std::unordered_map<Str, TextAction> TextInputPane::Impl_::s_action_map_;

TextInputPane::Impl_::Impl_(ContainerPane &root_pane, float padding) :
    root_pane_(root_pane),
    padding_(padding) {

    // Set up the static action map if not already done.
    if (s_action_map_.empty())
        InitActionMap_();

    // Find all the parts.
    text_pane_  = root_pane_.FindTypedPane<TextPane>("TextPane");
    cursor_     = SG::FindNodeUnderNode(root_pane_, "Cursor");
    selection_  = SG::FindNodeUnderNode(root_pane_, "Selection");
    background_ = SG::FindNodeUnderNode(root_pane_, "Background");
    auto widget = SG::FindTypedNodeUnderNode<GenericWidget>(
        root_pane_, "Widget");

    // Set up the widget used to activate the Pane, move the cursor, and drag
    // select.
    widget->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ ProcessClick_(info); });
    widget->GetDragged().AddObserver(
        this, [&](const DragInfo *info, bool is_start){
            ProcessDrag_(info, is_start); });
}

void TextInputPane::Impl_::SetInitialText(const Str &text) {
    initial_text_ = text;
    stack_.Init();
    SetText_(text);
    if (char_width_ > 0) {
        ClearSelection_();
        MoveCursorTo_(GetState_().GetCharCount());
    }
}

Str TextInputPane::Impl_::GetText() const {
    return GetState_().GetText();
}

void TextInputPane::Impl_::Activate() {
    if (! is_active_) {
        is_active_ = true;
        ASSERTM(char_width_ > 0, "In " + root_pane_.GetDesc());
        UpdateFromState_();

        if (virtual_keyboard_)
            AttachToVirtualKeyboard_();
    }
}

void TextInputPane::Impl_::Deactivate() {
    if (virtual_keyboard_)
        DetachFromVirtualKeyboard_();

    // If the current text is not valid, undo everything. Do this while still
    // considered active so the text updates.
    const Str text = GetState_().GetText();
    bool is_valid = ! validation_func_ || validation_func_(text);
    if (! is_valid) {
        while (stack_.Undo())
            ;
        UpdateFromState_();
    }

    is_active_ = false;
    UpdateFromState_();
}

bool TextInputPane::Impl_::HandleEvent(const Event &event) {
    bool ret = false;

    /// \todo Handle drag selection.

    if (is_active_) {
        if (event.flags.Has(Event::Flag::kKeyPress)) {
            // Check first for a key sequence in the action map.
            const Str key_string = event.GetKeyString();
            const auto it = s_action_map_.find(key_string);
            if (it != s_action_map_.end()) {
                auto act = it->second;
                ProcessAction_(act);
                // Do NOT return true for kAccept and kCancel actions; they may
                // also be handled elsewhere.
                ret = act != TextAction::kAccept && act != TextAction::kCancel;
            }

            // Otherwise, insert the text if it is text.
            else if (! event.key_text.empty()) {
                InsertChars_(event.key_text);
                ret = true;
            }

            stack_.Validate();
        }

        // If any change was made, make sure the cursor position is in sync
        // with the TextPane.
        if (ret)
            MoveCursorTo_(GetState_().GetCursorPos());
    }

    return ret;
}

void TextInputPane::Impl_::InitActionMap_() {
    s_action_map_["Backspace"]      = TextAction::kDeleteSelected;
    s_action_map_["Ctrl-Backspace"] = TextAction::kDeleteAll;
    s_action_map_["Ctrl-a"]         = TextAction::kSelectAll;
    s_action_map_["Ctrl-b"]         = TextAction::kMovePrevious;
    s_action_map_["Ctrl-d"]         = TextAction::kDeleteNext;
    s_action_map_["Ctrl-e"]         = TextAction::kMoveToEnd;
    s_action_map_["Ctrl-f"]         = TextAction::kMoveNext;
    s_action_map_["Ctrl-h"]         = TextAction::kDeletePrevious;
    s_action_map_["Ctrl-k"]         = TextAction::kDeleteToEnd;
    s_action_map_["Ctrl-z"]         = TextAction::kUndo;
    s_action_map_["Down"]           = TextAction::kMoveToEnd;
    s_action_map_["Enter"]          = TextAction::kAccept;
    s_action_map_["Escape"]         = TextAction::kCancel;
    s_action_map_["Left"]           = TextAction::kMovePrevious;
    s_action_map_["Right"]          = TextAction::kMoveNext;
    s_action_map_["Shift-Ctrl-a"]   = TextAction::kSelectNone;
    s_action_map_["Shift-Ctrl-k"]   = TextAction::kDeleteToStart;
    s_action_map_["Shift-Ctrl-z"]   = TextAction::kRedo;
    s_action_map_["Shift-Down"]     = TextAction::kSelectToEnd;
    s_action_map_["Shift-Left"]     = TextAction::kSelectPrevious;
    s_action_map_["Shift-Right"]    = TextAction::kSelectNext;
    s_action_map_["Shift-Up"]       = TextAction::kSelectToStart;
    s_action_map_["Up"]             = TextAction::kMoveToStart;
}

void TextInputPane::Impl_::AttachToVirtualKeyboard_() {
    ASSERT(virtual_keyboard_);
    auto &vk = *virtual_keyboard_;
    vk.GetInsertion().AddObserver(this, [&](const Str &s){ InsertChars_(s); });
    vk.GetAction().AddObserver(
        this, [&](TextAction action){ ProcessAction_(action); });
    vk.SetIsActive(true);
}

void TextInputPane::Impl_::DetachFromVirtualKeyboard_() {
    ASSERT(virtual_keyboard_);
    auto &vk = *virtual_keyboard_;
    vk.SetIsActive(false);
    vk.GetInsertion().RemoveObserver(this);
    vk.GetAction().RemoveObserver(this);
}

void TextInputPane::Impl_::SetText_(const Str &text) {
    GetState_().SetText(text);

    ASSERT(text_pane_);
    text_pane_->SetText(text);
    UpdateBackgroundColor_();
}

void TextInputPane::Impl_::PushText_(const Str &new_text) {
    stack_.Push();
    SetText_(new_text);
}

void TextInputPane::Impl_::Finish_(bool is_accept) {
    if (! is_accept)  // Cancel
        while (stack_.Undo())
            ;
    Deactivate();
}

void TextInputPane::Impl_::ProcessAction_(TextAction action) {
    //
    // Some rules:
    //  - Changing the cursor position always clears the current selection.
    //  - Changing the selection can also change the cursor position.
    //  - Inserting or deleting text always clears the current selection;
    //     inserting replaces the current selection.
    //  - Undo/redo restores the text and selection.
    //

    switch (action) {
        using enum TextAction;
      case kInsert:
        ASSERTM(false, "kInsert should use InsertText");
        break;

      case kToggleShift:  // No-op.
        break;

      case kDeleteAll:
      case kDeleteNext:
      case kDeletePrevious:
      case kDeleteSelected:
      case kDeleteToEnd:
      case kDeleteToStart:
        DeleteRange_(GetRangeForAction_(action));
        break;

      case kMoveNext:
        MoveCursorBy_(false, 1);
        break;
      case kMovePrevious:
        MoveCursorBy_(true, 1);
        break;
      case kMoveToEnd:
        MoveCursorBy_(false, 0);
        break;
      case kMoveToStart:
        MoveCursorBy_(true, 0);
        break;

      case kSelectAll:
      case kSelectNext:
      case kSelectNone:
      case kSelectPrevious:
      case kSelectToEnd:
      case kSelectToStart:
        ChangeSelection_(GetRangeForAction_(action));
        break;

      case kUndo:
        if (stack_.Undo())
            UpdateFromState_();
        break;
      case kRedo:
        if (stack_.Redo())
            UpdateFromState_();
        break;

      case kAccept:
        Finish_(true);
        break;
      case kCancel:
        Finish_(false);
        break;
    }
}

TextInputPane::Range_
TextInputPane::Impl_::GetRangeForAction_(TextAction action) const {
    const State_ &state = GetState_();
    Range_ range;

    switch (action) {
      case TextAction::kDeleteAll:
        range = state.GetFullRange();
        break;
      case TextAction::kDeleteNext:
        range = state.GetRangeRightOfCursor(1);
        break;
      case TextAction::kDeletePrevious:
        range = state.GetRangeLeftOfCursor(1);
        break;
      case TextAction::kDeleteSelected:
        range = state.HasSelection() ? state.GetSelectionRange() :
            state.GetRangeLeftOfCursor(1);
        break;
      case TextAction::kDeleteToEnd:
        range = state.GetRangeRightOfCursor(0);
        break;
      case TextAction::kDeleteToStart:
        range = state.GetRangeLeftOfCursor(0);
        break;

      case TextAction::kSelectAll:
        range = state.GetFullRange();
        break;
      case TextAction::kSelectNext:
        range = state.GetModifiedSelectionRange(false, 1);
        break;
      case TextAction::kSelectNone:
        break;  // Leave range empty.
      case TextAction::kSelectPrevious:
        range = state.GetModifiedSelectionRange(true, 1);
        break;
      case TextAction::kSelectToEnd:
        range = state.GetModifiedSelectionRange(false, 0);
        break;
      case TextAction::kSelectToStart:
        range = state.GetModifiedSelectionRange(true, 0);
        break;

      default:  // LCOV_EXCL_START [cannot happen]
        ASSERTM(false, "Invalid action: " + Util::EnumName(action));
        // LCOV_EXCL_STOP
    }

    return range;
}

void TextInputPane::Impl_::DeleteRange_(const Range_ &range) {
    // Ignore empty ranges.
    if (! range.IsEmpty()) {
        Str text = GetState_().GetText();
        ASSERT(range.end <= text.size());
        text.erase(text.begin() + range.start, text.begin() + range.end);

        PushText_(text);
        ClearSelection_();
        MoveCursorTo_(range.start);
    }
}

void TextInputPane::Impl_::InsertChars_(const Str &chars) {
    const State_ &state = GetState_();
    Str text = state.GetText();

    size_t insert_pos = state.GetCursorPos();

    // If there is anything selected, delete it and use its start as the insert
    // position.
    const Range_ sel_range = state.GetSelectionRange();
    if (! sel_range.IsEmpty()) {
        text.erase(text.begin() + sel_range.start,
                   text.begin() + sel_range.end);
        insert_pos = sel_range.start;
    }

    ASSERT(insert_pos <= text.size());
    text.insert(insert_pos, chars);
    PushText_(text);
    ClearSelection_();
    MoveCursorTo_(insert_pos + chars.size());
}

void TextInputPane::Impl_::ChangeSelection_(const Range_ &range) {
    const State_ &state = GetState_();
    const Range_ old_range = state.GetSelectionRange();

    if (range.start != old_range.start || range.end != old_range.end) {
        SetSelectionRange_(range);

        // Update the cursor. If the previous range was empty, move to the end
        // that is not the same as the cursor position. Otherwise, move to the
        // changing end.
        const size_t cursor_pos = state.GetCursorPos();
        if (old_range.IsEmpty())
            MoveCursorTo_(cursor_pos == range.start ? range.end : range.start);
        else
            MoveCursorTo_(range.start != old_range.start ?
                          range.start : range.end);
    }
}

void TextInputPane::Impl_::SetSelectionRange_(const Range_ &range) {
    if (range.IsEmpty()) {
        GetState_().SetSelectionRange(Range_());
        selection_->SetEnabled(false);
    }
    else {
        ASSERT(range.start < range.end);
        GetState_().SetSelectionRange(range);

        const float x0 = CharPosToX_(range.start);
        const float x1 = CharPosToX_(range.end);
        selection_->SetScale(Vector3f(x1 - x0, 1, 1));
        selection_->SetTranslation(Vector3f(.5f * (x0 + x1), 0,
                                            TK::kPaneZOffset));
        selection_->SetEnabled(true);
    }
}

void TextInputPane::Impl_::MoveCursorBy_(bool is_left, size_t count) {
    // If there is a current selection, move relative to one of the endpoints
    // (unless count is 0) and clear the selection. Otherwise, just move
    // relative to the current cursor position.
    const Range_ range = GetState_().GetSelectionRange();
    ClearSelection_();
    if (! range.IsEmpty() && count > 0)
        MoveCursorTo_(is_left ? range.start : range.end);
    else
        MoveCursorTo_(GetState_().GetCursorRelativePosition(is_left, count));
}

void TextInputPane::Impl_::MoveCursorTo_(size_t new_pos) {
    ASSERT(new_pos <= GetState_().GetCharCount());
    GetState_().SetCursorPos(new_pos);

    // Set the cursor position. Move it forward a little.
    const float x = CharPosToX_(new_pos);
    cursor_->SetTranslation(Vector3f(x, 0, .1f));
}

void TextInputPane::Impl_::UpdateFromState_() {
    if (is_active_) {
        // Update the text, cursor, and selection from the State_.
        const State_ &state = GetState_();
        SetText_(state.GetText());

        MoveCursorTo_(state.GetCursorPos());
        SetSelectionRange_(state.GetSelectionRange());
    }

    // Always update the background color.
    UpdateBackgroundColor_();

    // Turn the cursor and selection feedback on or off.
    ShowCursorAndSelection_(is_active_);
}

void TextInputPane::Impl_::UpdateCharWidth_() {
    ASSERT(text_pane_);

    // This uses a monospace font, so each character should be the same width.
    char_width_ = text_pane_->GetTextSize()[0] / GetState_().GetCharCount();
    ASSERT(char_width_ > 0);

    // Also undo the effects of scaling on the cursor.
    const float xscale =
        TK::kTextInputPaneCursorWidth / text_pane_->GetLayoutSize()[0];
    cursor_->SetScale(Vector3f(xscale, 1, 1));
}

void TextInputPane::Impl_::UpdateBackgroundColor_() {
    // Have to wait for Ion to be set up first.
    if (root_pane_.GetIonNode()) {
        Str color_name;
        if (is_active_) {
            const Str text = GetState_().GetText();
            is_valid_ = ! validation_func_ || validation_func_(text);
            color_name = is_valid_ ?
                "TextInputActiveColor" : "TextInputErrorColor";
        }
        else {
            color_name = "TextInputInactiveColor";
        }

        background_->SetBaseColor(SG::ColorMap::SGetColor(color_name));
    }
}

void TextInputPane::Impl_::ShowCursorAndSelection_(bool show) {
    cursor_->SetEnabled(show);
    selection_->SetEnabled(show && GetState_().HasSelection());
}

void TextInputPane::Impl_::ProcessClick_(const ClickInfo &info) {
    // If the pane is already active, adjust the cursor position.
    if (is_active_) {
        ClearSelection_();
        MoveCursorTo_(XToCharPos_(info.hit.point[0]));
    }
}

void TextInputPane::Impl_::ProcessDrag_(const DragInfo *info, bool is_start) {
    // Drags affect selection only when active. Note that info is null when the
    // drag ends.
    if (is_active_ && info) {
        // Compute the X coordinate of the drag point in object coordinates of
        // the TextInputPane.
        float x;
        if (info->trigger == Trigger::kPointer) {
            // The SG::Hit point is already in the correct coordinates.
            x = info->hit.point[0];
        }
        else {
            // The touch point needs to be converted from world coordinates.
            ASSERT(info->trigger == Trigger::kTouch);
            const SG::CoordConv cc(info->path_to_widget);
            const Point3f pane_pt = cc.RootToObject(info->touch_position);
            x = pane_pt[0];
        }

        // Convert the X coordinate to a character position and process the
        // selection change.
        const size_t pos = XToCharPos_(x);

        if (is_start) {
            ClearSelection_();
            drag_sel_start_ = pos;
        }
        else if (info) {
            if (pos == drag_sel_start_) {
                ClearSelection_();
                MoveCursorTo_(pos);
            }
            else {
                ChangeSelection_(Range_(std::min(drag_sel_start_, pos),
                                        std::max(drag_sel_start_, pos)));
            }
        }
    }
}

float TextInputPane::Impl_::CharPosToX_(size_t pos) const {
    ASSERT(root_pane_.GetLayoutSize()[0] > 0);

    // The X value ranges from -.5 to +.5 across the TextInputPane.  The text
    // starts just after the padding, so start there and add the appropriate
    // number of character widths.
    return -.5f +
        (padding_ + pos * char_width_) / root_pane_.GetLayoutSize()[0];
}

size_t TextInputPane::Impl_::XToCharPos_(float x) const {
    ASSERT(char_width_ > 0);

    // The math here is the inverse of CharPosToX_().
    const float pane_width = root_pane_.GetLayoutSize()[0];
    const float pos = ((x + .5f) * pane_width - padding_) / char_width_;

    // Round and clamp to the number of characters in the text string.
    return static_cast<size_t>(Clamp(pos + .5f,
                                     0.f, GetState_().GetCharCount()));
}

// ----------------------------------------------------------------------------
// TextInputPane functions.
// ----------------------------------------------------------------------------

TextInputPane::TextInputPane() {
}

void TextInputPane::AddFields() {
    AddField(initial_text_.Init("initial_text", "."));

    BoxPane::AddFields();
}

void TextInputPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate()) {
        impl_.reset(new Impl_(*this, GetPadding()));
        impl_->SetInitialText(initial_text_);
    }
}

void TextInputPane::SetValidationFunc(const ValidationFunc &func) {
    impl_->SetValidationFunc(func);
}

void TextInputPane::SetInitialText(const Str &text) {
    impl_->SetInitialText(text);
}

Str TextInputPane::GetText() const {
    return impl_->GetText();
}

bool TextInputPane::IsTextValid() const {
    return impl_->IsTextValid();
}

void TextInputPane::SetLayoutSize(const Vector2f &size) {
    const float cur_width = GetLayoutSize()[0];
    BoxPane::SetLayoutSize(size);
    const float new_width = GetLayoutSize()[0];
    if (new_width != cur_width)
        impl_->UpdateForWidthChange();
}

IPaneInteractor * TextInputPane::GetInteractor() {
    return impl_.get();
}
