#pragma once

#include "Base/Memory.h"
#include "Enums/TextAction.h"
#include "Util/Notifier.h"

// VirtualKeyboard is shared in several places.
DECL_SHARED_PTR(VirtualKeyboard);

/// The VirtualKeyboard class manages a virtual keyboard used for input to
/// text-based GUI elements when in VR with the headset on. It is used to
/// connect KeyboardPanel key presses to TextInputPane edits via callbacks.
///
/// \ingroup Base
class VirtualKeyboard {
  public:
    typedef std::function<void(bool)> ShowHideFunc;

    /// Sets a function to invoke that implements whatever is necessary to show
    /// or hide the VirtualKeyboard. It is passed a flag that is true for show
    /// and false for hide. This function will be invoked with true when the
    /// VirtualKeyboard is set to be both active and visible and with false
    /// when both flags are no longer true.
    void SetShowHideFunc(const ShowHideFunc &func) { show_hide_func_ = func; }

    /// Sets a flag indicating whether the VirtualKeyboard is active, meaning
    /// that it is currently attached to an active TextInputPane.
    void SetIsActive(bool flag);

    /// Returns a flag indicating whether the VirtualKeyboard is active,
    /// meaning that it is currently attached to an active TextInputPane.
    bool IsActive() const { return is_active_; }

    /// Sets a flag indicating whether the VirtualKeyboard is visible, meaning
    /// that the Board the KeyboardPanel is attached to is currently shown.
    void SetIsVisible(bool flag);

    /// Returns a flag indicating whether the VirtualKeyboard is visible,
    /// meaning that the Board the KeyboardPanel is attached to is currently
    /// shown.
    bool IsVisible() const { return is_visible_; }

    /// Inserts the given string into the current text.
    void InsertText(const std::string &s);

    /// Processes the given TextAction. (The action should not be
    /// TextAction::kInsert, which should use the InsertText() function.)
    void ProcessTextAction(TextAction action);

    /// Returns a Notifier that is invoked when the user touches a key on the
    /// VirtualKeyboard that inserts one or more characters. A string
    /// containing the character(s) is supplied.
    Util::Notifier<const std::string &> & GetInsertion() { return insertion_; }

    /// Returns a Notifier that is invoked when the user presses any key that
    /// performs an action other than insertion.
    Util::Notifier<TextAction> & GetAction() { return action_; }

  private:
    ShowHideFunc show_hide_func_;
    bool         is_active_  = false;
    bool         is_visible_ = false;

    /// Notifies when the user inserts one or more characters.
    Util::Notifier<const std::string &> insertion_;
    /// Notifies when the user performs some other action.
    Util::Notifier<TextAction>          action_;

    /// Returns true if the VirtualKeyboard is shown and should therefore
    /// notify any of its observers.
    bool IsShown_() const { return IsVisible() && IsActive(); }
};
