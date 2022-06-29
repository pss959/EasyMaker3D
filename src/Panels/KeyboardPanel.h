#pragma once

#include <string>
#include <vector>

#include "Base/Memory.h"
#include "Panels/Panel.h"

DECL_SHARED_PTR(KeyPane);
DECL_SHARED_PTR(KeyboardPanel);
DECL_SHARED_PTR(Pane);

namespace Parser { class Registry; }

/// KeyboardPanel is a derived Panel class that acts as a virtual keyboard.
///
/// \ingroup Panels
class KeyboardPanel : public Panel {
  protected:
    KeyboardPanel() {}

    virtual void InitInterface() override;

  private:
    /// Stores all KeyPane instances in the KeyboardPanel.
    std::vector<KeyPanePtr> key_panes_;

    /// Recursively adds all KeyPane instances to the key_panes_ vector.
    void FindKeyPanes_(const PanePtr &pane);

    /// KeyPane button press callback.
    void ProcessKey_(const KeyPane &key_pane);

    /// Special case for handling shift key presses.
    void ProcessShiftKey_(const KeyPane &shift_key_pane);

    friend class Parser::Registry;
};
