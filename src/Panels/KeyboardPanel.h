#pragma once

#include <string>
#include <vector>

#include "Base/Memory.h"
#include "Panels/Panel.h"

class VirtualKeyboard;

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(KeyboardPanel);
DECL_SHARED_PTR(Pane);
DECL_SHARED_PTR(TextPane);

namespace Parser { class Registry; }

/// KeyboardPanel is a derived Panel class that acts as a virtual keyboard.
///
/// \ingroup Panels
class KeyboardPanel : public Panel {
  public:
    /// Returns the VirtualKeyboard used by the KeyboardPanel.
    VirtualKeyboard & GetVirtualKeyboard() { return *virtual_keyboard_; }

  protected:
    KeyboardPanel();

    virtual void InitInterface() override;
    virtual bool HandleEvent(const Event &event) override;

    /// Redefines this to set up the VirtualKeyboard if the Pane is a
    /// TextInputPane.
    virtual void PaneActivated(Pane &pane) override;

    /// Redefines this to disable the VirtualKeyboard if the Pane is a
    /// TextInputPane.
    virtual void PaneDeactivated(Pane &pane) override;

  private:
    struct Key_ {
        ButtonPanePtr button_pane;
        TextPanePtr   text_pane;
        std::string   text;
        std::string   shifted_text;
    };
    std::vector<Key_> keys_;

    size_t shift_count_ = 0;  ///< Number of active shift keys.

    /// VirtualKeyboard used when in VR with the headset on.
    std::unique_ptr<VirtualKeyboard> virtual_keyboard_;

    /// Recursively adds all ButtonPane instances to the vector.
    void FindButtonPanes_(const PanePtr &pane,
                          std::vector<ButtonPanePtr> &button_panes);

    /// Button callback for shift key.
    void ProcessShiftKey_(const Key_ &key);

    /// Button callback for all other keys.
    void ProcessKey_(const Key_ &key);

    /// Updates the text on all keys for a changing shift state.
    void UpdateKeyText_(bool is_shifted);

    /// Returns the shifted version for the given key text.
    static std::string GetShiftedText_(const std::string &s);

    friend class Parser::Registry;
};
