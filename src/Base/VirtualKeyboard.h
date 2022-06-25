#pragma once

#include "Util/Notifier.h"

/// The VirtualKeyboard class manages a virtual keyboard used for input to
/// text-based GUI elements when in VR with the headset on. It is used to
/// connect KeyboardPanel key presses to TextInputPane edits via callbacks.
///
/// \ingroup Base
class VirtualKeyboard {
  public:
    /// Sets a flag indicating whether the VirtualKeyboard is active, meaning
    /// that it is currently attached to an active TextInputPane.
    void SetIsActive(bool flag) { is_active_ = flag; }

    /// Returns a flag indicating whether the VirtualKeyboard is active,
    /// meaning that it is currently attached to an active TextInputPane.
    bool IsActive() const { return is_active_; }

    /// Sets a flag indicating whether the VirtualKeyboard is visible, meaning
    /// that the Board the KeyboardPanel is attached to is currently shown.
    void SetIsVisible(bool flag) { is_visible_ = flag; }

    /// Returns a flag indicating whether the VirtualKeyboard is visible,
    /// meaning that the Board the KeyboardPanel is attached to is currently
    /// shown.
    bool IsVisible() const { return is_visible_; }

    /// Inserts the given string into the current text.
    void InsertText(const std::string &s);

    /// Deletes the character before the insertion point, if any.
    void DeletePreviousChar();

    /// Clears the current text.
    void ClearText();

    /// Indicates that the VirtualKeyboard is done. A flag indicating whether
    /// to accept the new text or cancel is supplied.
    void Finish(bool accept);

    /// Returns a Notifier that is invoked when the user touches a key on the
    /// VirtualKeyboard that inserts one or more characters. A string
    /// containing the character(s) is supplied.
    Util::Notifier<const std::string &> & GetInsertion() { return insertion_; }

    /// Returns a Notifier that is invoked when the user touches a key on the
    /// VirtualKeyboard that deletes characters. A flag indicating whether to
    /// delete the entire string or just the character before the insertion
    /// point is supplied.
    Util::Notifier<bool> & GetDeletion() { return deletion_; }

    /// Returns a Notifier that is invoked when the user touches the Accept or
    /// Cancel buttons on the VirtualKeyboard. A flag indicating whether to
    /// accept the result is supplied.
    Util::Notifier<bool> & GetCompletion() { return completion_; }

  private:
    bool is_active_  = false;
    bool is_visible_ = false;

    /// Notifies when the user inserts one or more characters.
    Util::Notifier<const std::string &> insertion_;
    /// Notifies when the user deletes a character or the entire text.
    Util::Notifier<bool>                deletion_;
    /// Notifies when the user hits the Accept or Cancel buttons.
    Util::Notifier<bool>                completion_;

    /// Returns true if the VirtualKeyboard should notify any of its
    /// observers.
    bool ShouldNotify_() const { return IsVisible() && IsActive(); }
};
