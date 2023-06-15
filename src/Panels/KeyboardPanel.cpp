#include "Panels/KeyboardPanel.h"

#include <vector>

#include "Base/VirtualKeyboard.h"
#include "Panes/ButtonPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/KeyPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "Util/Assert.h"
#include "Widgets/PushButtonWidget.h"

void KeyboardPanel::InitInterface() {
    FindKeyPanes_(GetPane());

    // Set up a click callback for each KeyPane.
    for (auto &key_pane: key_panes_) {
        key_pane->GetButton().GetClicked().AddObserver(
            this, [&](const ClickInfo &){ ProcessKey_(*key_pane); });
    }
}

void KeyboardPanel::FindKeyPanes_(const PanePtr &pane) {
    if (KeyPanePtr key_pane = std::dynamic_pointer_cast<KeyPane>(pane))
        key_panes_.push_back(key_pane);

    // Recurse if this is a ContainerPane.
    if (ContainerPanePtr ctr = std::dynamic_pointer_cast<ContainerPane>(pane)) {
        for (const auto &sub_pane: ctr->GetPanes())
            FindKeyPanes_(sub_pane);
    }
}

void KeyboardPanel::ProcessKey_(const KeyPane &key_pane) {
    std::string chars;
    const auto action = key_pane.GetAction(chars);

    // Special case for shift keys, which are toggles that affect the state of
    // the keys but nothing else.
    if (action == TextAction::kToggleShift) {
        ProcessShiftKey_(key_pane);
    }

    // All of the other actions require a VirtualKeyboard.
    else {
        auto &vk = GetContext().virtual_keyboard;
        if (vk) {
            if (action == TextAction::kInsert) {
                // Insert characters.
                ASSERT(! chars.empty());
                vk->InsertText(chars);
            }
            else {
                // Any other action.
                vk->ProcessTextAction(action);
            }
        }
    }
}

void KeyboardPanel::ProcessShiftKey_(const KeyPane &shift_key_pane) {
    const bool is_shifted = shift_key_pane.GetButton().GetToggleState();

    std::string chars;
    for (auto &key_pane: key_panes_) {
        // Update the toggle state for other shift keys.
        if (key_pane->GetAction(chars) == TextAction::kToggleShift) {
            if (key_pane.get() != &shift_key_pane)
                key_pane->GetButton().SetToggleState(is_shifted);
        }

        // Update all other keys with the new state.
        else {
            key_pane->ProcessShift(is_shifted);
        }
    }
}
