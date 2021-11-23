#include "ShortcutHandler.h"

#include <iostream>

#include "Event.h"

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {

        const std::string key_string = event.GetKeyString();

        // Ctrl-Q: quit!
        if (key_string == "<Ctrl>q") {
            action_manager_->ApplyAction(Action::kQuit);
            return true;
        }

#if DEBUG
        // Ctrl-B: Print bounds.
        if (key_string == "<Ctrl>b") {
            action_manager_->ApplyAction(Action::kPrintBounds);
            return true;
        }

        // Ctrl-F: Print panes in floating board.
        if (key_string == "<Ctrl>f") {
            action_manager_->ApplyAction(Action::kPrintPanes);
            return true;
        }

        // Ctrl-M: Print matrices.
        if (key_string == "<Ctrl>m") {
            action_manager_->ApplyAction(Action::kPrintMatrices);
            return true;
        }

        // Ctrl-N: Print nodes and shapes.
        if (key_string == "<Ctrl>n") {
            action_manager_->ApplyAction(Action::kPrintNodesAndShapes);
            return true;
        }

        // Ctrl-P: Print scene contents.
        if (key_string == "<Ctrl>p") {
            action_manager_->ApplyAction(Action::kPrintScene);
            return true;
        }

        // Ctrl-R: Reload the scene.
        else if (key_string == "<Ctrl>r") {
            action_manager_->ApplyAction(Action::kReloadScene);
            return true;
        }

        // Ctrl-Z: Undo.
        else if (key_string == "<Ctrl>z") {
            if (action_manager_->CanApplyAction(Action::kUndo))
                action_manager_->ApplyAction(Action::kUndo);
            return true;
        }
        // Shift-Ctrl-Z: Redo.
        else if (key_string == "<Shift><Ctrl>z") {
            if (action_manager_->CanApplyAction(Action::kRedo))
                action_manager_->ApplyAction(Action::kRedo);
            return true;
        }
#endif
    }
    return false;
}
