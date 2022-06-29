#pragma once

#include "Base/Memory.h"
#include "Panes/ButtonPane.h"

DECL_SHARED_PTR(KeyPane);

/// KeyPane is an abstract base class derived from ButtonPane that is used
/// exclusively in the KeyboardPane for defining keys on the virtual keyboard.
///
/// \ingroup Panes
class KeyPane : public ButtonPane {
  public:
    /// Actions that can be performed when a KeyPane is activated.
    enum class Action {
        kInsertChars,   ///< Insert the "text" character(s).
        kBackspace,     ///< Delete the previous character, if any.
        kClear,         ///< Clear all characters.
        kMoveNext,      ///< Move the cursor to the next character.
        kMovePrevious,  ///< Move the cursor to the previous character.
        kMoveToEnd,     ///< Move the cursor to the end of the text.
        kMoveToStart,   ///< Move the cursor to the start of the text.
        kToggleShift,   ///< Toggles the shift state.
        kAccept,        ///< Close the keyboard, accepting the current text.
        kCancel,        ///< Close the keyboard, restoring the previous text.
    };

    /// Returns the action to perform.
    virtual Action GetAction() const = 0;

  protected:
    KeyPane() {}
};
