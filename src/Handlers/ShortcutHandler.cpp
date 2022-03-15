#include "ShortcutHandler.h"

#include "Debug/Print.h"
#include "Event.h"
#include "Util/KLog.h"

ShortcutHandler::ShortcutHandler() {
    // Populate the map from event key strings to actions.
    auto add_shortcut = [&](const std::string &key, Action action){
        action_map_[key] = action;
    };

    add_shortcut(" ",           Action::kToggleSpecializedTool);
    add_shortcut("<",           Action::kDecreaseComplexity);
    add_shortcut("<Ctrl>A",     Action::kSelectNone);
    add_shortcut("<Ctrl>Down",  Action::kSelectFirstChild);
    add_shortcut("<Ctrl>H",     Action::kShowAll);
    add_shortcut("<Ctrl>I",     Action::kOpenInfoPanel);
    add_shortcut("<Ctrl>Left",  Action::kSelectPreviousSibling);
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
    add_shortcut("<Ctrl>p",     Action::kOpenSettingsPanel);
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

#if DEBUG
    add_shortcut("<Alt>r", Action::kReloadScene);
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

        if (key_string == "<Alt>!")
            KLogger::ToggleLogging();
#endif
    }
    return false;
}
