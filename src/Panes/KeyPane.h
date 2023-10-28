#pragma once

#include "Enums/TextAction.h"
#include "Panes/ButtonPane.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(KeyPane);

/// KeyPane is an abstract base class derived from ButtonPane that is used
/// exclusively in the KeyboardPane for defining keys on the virtual keyboard.
///
/// \ingroup Panes
class KeyPane : public ButtonPane {
  public:
    /// Returns the action to perform. If the action is TextAction::kInsert,
    /// the characters to insert are stored in chars, which is otherwise left
    /// alone.
    virtual TextAction GetAction(Str &chars) const = 0;

    /// Responds to a change in the current shift state. The base class defines
    /// this to do nothing.
    virtual void ProcessShift(bool is_shifted) {}

  protected:
    KeyPane() {}
};
