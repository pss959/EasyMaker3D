#include "ShortcutHandler.h"

#include <iostream>

#include "Event.h"

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {

        // Escape key or Ctrl-Q: quit!
        if (event.key_string == "Escape" || event.key_string == "<Ctrl>q") {
            action_manager_->ApplyAction(Action::kQuit);
            return true;
        }

#if DEBUG
        // Ctrl-B: Print bounds.
        if (event.key_string == "<Ctrl>b") {
            action_manager_->ApplyAction(Action::kPrintBounds);
            return true;
        }

        // Ctrl-M: Print matrices.
        if (event.key_string == "<Ctrl>m") {
            action_manager_->ApplyAction(Action::kPrintMatrices);
            return true;
        }

        // Ctrl-P: Print scene contents.
        if (event.key_string == "<Ctrl>p") {
            action_manager_->ApplyAction(Action::kPrintScene);
            return true;
        }

        // Ctrl-R: Reload the scene.
        else if (event.key_string == "<Ctrl>r") {
            action_manager_->ApplyAction(Action::kReloadScene);
            return true;
        }

        // Ctrl-Z: Undo.
        else if (event.key_string == "<Ctrl>z") {
            if (action_manager_->CanApplyAction(Action::kUndo))
                action_manager_->ApplyAction(Action::kUndo);
            return true;
        }
        // Shift-Ctrl-Z: Redo.
        else if (event.key_string == "<Shift><Ctrl>z") {
            if (action_manager_->CanApplyAction(Action::kRedo))
                action_manager_->ApplyAction(Action::kRedo);
            return true;
        }
#endif
    }
    return false;
}
