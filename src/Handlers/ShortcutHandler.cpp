#include "Handlers/ShortcutHandler.h"

#include "Base/Event.h"
#include "Debug/Print.h"
#include "Managers/ActionManager.h"
#include "Util/KLog.h"

ShortcutHandler::ShortcutHandler() {
    // Populate the map from event key strings to actions.
    auto add_shortcut = [&](const std::string &key, Action action){
        action_map_[key] = action;
    };

    // Keyboard keys.
    add_shortcut(" ",           Action::kToggleSpecializedTool);
    add_shortcut("<",           Action::kDecreaseComplexity);
    add_shortcut("<Ctrl>,",     Action::kOpenSettingsPanel);
    add_shortcut("<Ctrl>A",     Action::kSelectNone);
    add_shortcut("<Ctrl>Down",  Action::kSelectFirstChild);
    add_shortcut("<Ctrl>E",     Action::kToggleEdgeTarget);
    add_shortcut("<Ctrl>H",     Action::kShowAll);
    add_shortcut("<Ctrl>I",     Action::kOpenInfoPanel);
    add_shortcut("<Ctrl>Left",  Action::kSelectPreviousSibling);
    add_shortcut("<Ctrl>P",     Action::kTogglePointTarget);
    add_shortcut("<Ctrl>Right", Action::kSelectNextSibling);
    add_shortcut("<Ctrl>Up",    Action::kSelectParent);
    add_shortcut("<Ctrl>V",     Action::kPasteInto);
    add_shortcut("<Ctrl>Z",     Action::kRedo);
    add_shortcut("<Ctrl>a",     Action::kSelectAll);
    add_shortcut("<Ctrl>b",     Action::kToggleBuildVolume);
    add_shortcut("<Ctrl>c",     Action::kCopy);
    add_shortcut("<Ctrl>e",     Action::kToggleShowEdges);
    add_shortcut("<Ctrl>h",     Action::kHideSelected);
    add_shortcut("<Ctrl>i",     Action::kCreateImportedModel);
    add_shortcut("<Ctrl>l",     Action::kToggleLeftRadialMenu);
    add_shortcut("<Ctrl>n",     Action::kNameTool);
    add_shortcut("<Ctrl>q",     Action::kQuit);
    add_shortcut("<Ctrl>r",     Action::kToggleRightRadialMenu);
    add_shortcut("<Ctrl>s",     Action::kOpenSessionPanel);
    add_shortcut("<Ctrl>t",     Action::kToggleInspector);
    add_shortcut("<Ctrl>v",     Action::kPaste);
    add_shortcut("<Ctrl>x",     Action::kCut);
    add_shortcut("<Ctrl>z",     Action::kUndo);
    add_shortcut("=",           Action::kMoveToOrigin);
    add_shortcut(">",           Action::kIncreaseComplexity);
    add_shortcut("?",           Action::kOpenHelpPanel);
    add_shortcut("Backspace",   Action::kDelete);
    add_shortcut("F1",          Action::kOpenHelpPanel);
    add_shortcut("[",           Action::kSwitchToPreviousTool);
    add_shortcut("]",           Action::kSwitchToNextTool);
    add_shortcut("x",           Action::kIncreasePrecision);
    add_shortcut("z",           Action::kDecreasePrecision);
    add_shortcut("{",           Action::kMovePrevious);
    add_shortcut("}",           Action::kMoveNext);
#if DEBUG
    add_shortcut("<Alt>r", Action::kReloadScene);
#endif

    // Controller buttons.
    add_shortcut("L:Menu",      Action::kToggleLeftRadialMenu);
    add_shortcut("R:Menu",      Action::kToggleRightRadialMenu);
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
        if (action_manager_->CanApplyAction(action))
            action_manager_->ApplyAction(action);
        return true;
    }

#if ENABLE_DEBUG_PRINT
    // Special cases for debugging shortcuts.
    if (Debug::ProcessPrintShortcut(str))
        return true;
    if (str == "<Alt>!") {
        KLogger::ToggleLogging();
        return true;
    }
#endif

    return false;
}
