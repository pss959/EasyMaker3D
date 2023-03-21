#include "Handlers/ShortcutHandler.h"

#include "Agents/ActionAgent.h"
#include "Base/Event.h"
#include "Debug/Shortcuts.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

ShortcutHandler::ShortcutHandler() {
    // Populate the map from event key strings to actions.
    auto add_shortcut = [&](const std::string &key, Action action){
        action_map_[key] = action;
    };

    // Keyboard keys.
    add_shortcut(" ",            Action::kToggleSpecializedTool);
    add_shortcut(",",            Action::kDecreaseComplexity);
    add_shortcut(".",            Action::kIncreaseComplexity);
    add_shortcut("=",            Action::kMoveToOrigin);
    add_shortcut("Backspace",    Action::kDelete);
    add_shortcut("Ctrl-,",       Action::kOpenSettingsPanel);
    add_shortcut("Ctrl-Down",    Action::kSelectFirstChild);
    add_shortcut("Ctrl-Left",    Action::kSelectPreviousSibling);
    add_shortcut("Ctrl-Right",   Action::kSelectNextSibling);
    add_shortcut("Ctrl-Up",      Action::kSelectParent);
    add_shortcut("Ctrl-a",       Action::kSelectAll);
    add_shortcut("Ctrl-b",       Action::kToggleBuildVolume);
    add_shortcut("Ctrl-c",       Action::kCopy);
    add_shortcut("Ctrl-e",       Action::kToggleShowEdges);
    add_shortcut("Ctrl-h",       Action::kHideSelected);
    add_shortcut("Ctrl-i",       Action::kCreateImportedModel);
    add_shortcut("Ctrl-l",       Action::kToggleLeftRadialMenu);
    add_shortcut("Ctrl-n",       Action::kNameTool);
    add_shortcut("Ctrl-q",       Action::kQuit);
    add_shortcut("Ctrl-r",       Action::kToggleRightRadialMenu);
    add_shortcut("Ctrl-s",       Action::kOpenSessionPanel);
    add_shortcut("Ctrl-t",       Action::kToggleInspector);
    add_shortcut("Ctrl-v",       Action::kPaste);
    add_shortcut("Ctrl-x",       Action::kCut);
    add_shortcut("Ctrl-z",       Action::kUndo);
    add_shortcut("Delete",       Action::kDelete);
    add_shortcut("F1",           Action::kOpenHelpPanel);
    add_shortcut("Shift-/",      Action::kOpenHelpPanel);
    add_shortcut("Shift-Ctrl-a", Action::kSelectNone);
    add_shortcut("Shift-Ctrl-e", Action::kToggleEdgeTarget);
    add_shortcut("Shift-Ctrl-h", Action::kShowAll);
    add_shortcut("Shift-Ctrl-i", Action::kOpenInfoPanel);
    add_shortcut("Shift-Ctrl-p", Action::kTogglePointTarget);
    add_shortcut("Shift-Ctrl-v", Action::kPasteInto);
    add_shortcut("Shift-Ctrl-z", Action::kRedo);
    add_shortcut("Shift-[",      Action::kMovePrevious);
    add_shortcut("Shift-]",      Action::kMoveNext);
    add_shortcut("[",            Action::kSwitchToPreviousTool);
    add_shortcut("]",            Action::kSwitchToNextTool);
    add_shortcut("x",            Action::kIncreasePrecision);
    add_shortcut("z",            Action::kDecreasePrecision);

    // Controller buttons.
    add_shortcut("L:Menu",       Action::kToggleLeftRadialMenu);
    add_shortcut("R:Menu",       Action::kToggleRightRadialMenu);
    add_shortcut("L:Center",     Action::kToggleSpecializedTool);
    add_shortcut("R:Center",     Action::kToggleSpecializedTool);
    add_shortcut("L:Up",         Action::kIncreasePrecision);
    add_shortcut("R:Up",         Action::kIncreasePrecision);
    add_shortcut("L:Down",       Action::kDecreasePrecision);
    add_shortcut("R:Down",       Action::kDecreasePrecision);
    add_shortcut("L:Left",       Action::kSwitchToPreviousTool);
    add_shortcut("R:Left",       Action::kSwitchToPreviousTool);
    add_shortcut("L:Right",      Action::kSwitchToNextTool);
    add_shortcut("R:Right",      Action::kSwitchToNextTool);

#if ! RELEASE_BUILD
    add_shortcut("Alt-r",        Action::kReloadScene);
#endif
}

bool ShortcutHandler::AddCustomShortcuts(std::string &error) {
    // XXXX
    //error = "XXXX Mysterious error";
    //return false;
    return true;
}

void ShortcutHandler::GetShortcutStrings(Action action,
                                         std::string &keyboard_string,
                                         std::string &controller_string) const {
    using ion::base::StartsWith;

    keyboard_string.clear();
    controller_string.clear();
    for (const auto &it: action_map_) {
        if (it.second == action) {
            const bool is_controller =
                StartsWith(it.first, "L:") || StartsWith(it.first, "R:");
            std::string &str =
                is_controller ? controller_string : keyboard_string;
            if (! str.empty())
                str += ", ";
            str += it.first;
        }
    }
}

bool ShortcutHandler::HandleEvent(const Event &event) {
    // Handle special key presses.
    const std::string key_string = event.GetKeyString();
    if (event.flags.Has(Event::Flag::kKeyPress) &&
        ! key_string.empty() && HandleShortcutString_(key_string))
        return true;

    // Handle special Controller button presses.
    const std::string cb_string = event.GetControllerButtonString();
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        ! cb_string.empty() && HandleShortcutString_(cb_string))
        return true;

    return false;
}

bool ShortcutHandler::HandleShortcutString_(const std::string &str) {
    auto it = action_map_.find(str);
    if (it != action_map_.end()) {
        const Action action = it->second;
        ASSERT(action_agent_);
        if (action_agent_->CanApplyAction(action))
            action_agent_->ApplyAction(action);
        return true;
    }

#if ENABLE_DEBUG_FEATURES
    // Special cases for debugging shortcuts.
    if (Debug::HandleShortcut(str))
        return true;
#endif

    return false;
}
