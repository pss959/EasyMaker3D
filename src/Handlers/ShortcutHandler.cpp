#include "ShortcutHandler.h"

#include "Debug/Print.h"
#include "Event.h"

ShortcutHandler::ShortcutHandler() {
    // Populate the map from event key strings to actions.
    action_map_["<Ctrl>i"] = Action::kOpenInfoPanel;
    action_map_["<Ctrl>q"] = Action::kQuit;
    action_map_["<Ctrl>z"] = Action::kUndo;
    action_map_["<Ctrl>Z"] = Action::kRedo;

#if DEBUG
    action_map_["<Ctrl>r"] = Action::kReloadScene;
#endif
}

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        auto it = action_map_.find(key_string);
        if (it != action_map_.end()) {
            const Action action = it->second;
            if (action_manager_->CanApplyAction(action))
                action_manager_->ApplyAction(action);
            return true;
        }
#if DEBUG
        if (Debug::ProcessPrintShortcut(key_string))
            return true;
#endif
    }
    return false;
}
