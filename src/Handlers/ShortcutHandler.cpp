#include "Handlers/ShortcutHandler.h"

#include <ion/base/stringutils.h>

#include "Agents/ActionAgent.h"
#include "Base/Event.h"
#include "Debug/Shortcuts.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"
#include "Util/Read.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// ShortcutHandler::CustomProcessor_ class.
// ----------------------------------------------------------------------------

/// The ShortcutHandler::Parser_ class manages parsing and processing of custom
/// shortcuts read from a file.
class ShortcutHandler::Parser_ {
  public:
    /// Processes shortcuts from the given file contents, adding to the given
    /// map. Sets error and returns false if anything goes wrong.
    bool AddShortcuts(const std::string &contents, ActionMap_ &action_map,
                      std::string &error);

  private:
    /// Current line number during parsing (starting at 1).
    int         line_number_ = 0;

    /// Stores any current error string.
    std::string error_;

    /// Parses a single line of input.
    bool ParseLine_(const std::string &line, ActionMap_ &action_map);

    /// Reports an error.
    bool Error_(const std::string &what) {
        error_ = what  + " on line " + Util::ToString(line_number_);
        return false;
    };
};

bool ShortcutHandler::Parser_::AddShortcuts(const std::string &contents,
                                            ActionMap_ &action_map,
                                            std::string &error) {
    error.clear();

    // Split into lines.
    for (auto &line: ion::base::SplitString(contents, "\n")) {
        ++line_number_;

        // Trim whitespace.
        line = ion::base::TrimStartAndEndWhitespace(line);

        // Skip blank lines and lines starting with '#'.
        if (line.empty() || line[0] == '#')
            continue;

        if (! ParseLine_(line, action_map)) {
            error = error_;
            return false;
        }
    }

    return true;
}

bool ShortcutHandler::Parser_::ParseLine_(const std::string &line,
                                          ActionMap_ &action_map) {
    using ion::base::SplitString;

    // Split into words.
    const std::vector<std::string> words = SplitString(line, " \t");
    if (words.size() != 2U)
        return Error_("Syntax error");

    // The shortcut key string comes first. Split it by "-".
    const std::vector<std::string> s = SplitString(words[0], "-");

    // All but the last part should be a valid modifier.
    Event::Modifiers modifiers;
    for (size_t i = 0; i + 1 < s.size(); ++i) {
        const std::string &mod = s[i];
        if (ion::base::CompareCaseInsensitive(mod, "shift") == 0)
            modifiers.Set(Event::ModifierKey::kShift);
        else if (ion::base::CompareCaseInsensitive(mod, "ctrl") == 0)
            modifiers.Set(Event::ModifierKey::kControl);
        else if (ion::base::CompareCaseInsensitive(mod, "alt") == 0)
            modifiers.Set(Event::ModifierKey::kAlt);
        else
            return Error_("Invalid key modifier: \"" + mod + "\"");
    }

    // The last part is assumed to be a valid key name. If it isn't, it won't
    // work. Leaving this unvalidated makes it possible to handle other (and
    // future) GLFW key names.

    // Build a key string from the modifiers and the key name.
    const std::string key_string = Event::BuildKeyString(modifiers, s.back());

    // The scond word is the action to perform. A valid action string has the
    // same name as the corresponding action enum without the leading "k".
    const std::string &action_name = words[1];
    Action action;
    if (! Util::EnumFromString("k" + action_name, action))
        return Error_("Invalid action name: \"" + action_name + "\"");

    action_map[key_string] = action;

    return true;
}

// ----------------------------------------------------------------------------
// ShortcutHandler functions.
// ----------------------------------------------------------------------------

ShortcutHandler::ShortcutHandler() {
    // Populate the map from event key strings to actions.

    // Keyboard keys.
    AddShortcut_(" ",            Action::kToggleSpecializedTool);
    AddShortcut_(",",            Action::kDecreaseComplexity);
    AddShortcut_(".",            Action::kIncreaseComplexity);
    AddShortcut_("=",            Action::kMoveToOrigin);
    AddShortcut_("Backspace",    Action::kDelete);
    AddShortcut_("Ctrl-,",       Action::kOpenSettingsPanel);
    AddShortcut_("Ctrl-Down",    Action::kSelectFirstChild);
    AddShortcut_("Ctrl-Left",    Action::kSelectPreviousSibling);
    AddShortcut_("Ctrl-Right",   Action::kSelectNextSibling);
    AddShortcut_("Ctrl-Up",      Action::kSelectParent);
    AddShortcut_("Ctrl-a",       Action::kSelectAll);
    AddShortcut_("Ctrl-b",       Action::kToggleBuildVolume);
    AddShortcut_("Ctrl-c",       Action::kCopy);
    AddShortcut_("Ctrl-e",       Action::kToggleShowEdges);
    AddShortcut_("Ctrl-h",       Action::kHideSelected);
    AddShortcut_("Ctrl-i",       Action::kCreateImportedModel);
    AddShortcut_("Ctrl-l",       Action::kToggleLeftRadialMenu);
    AddShortcut_("Ctrl-n",       Action::kNameTool);
    AddShortcut_("Ctrl-q",       Action::kQuit);
    AddShortcut_("Ctrl-r",       Action::kToggleRightRadialMenu);
    AddShortcut_("Ctrl-s",       Action::kOpenSessionPanel);
    AddShortcut_("Ctrl-t",       Action::kToggleInspector);
    AddShortcut_("Ctrl-v",       Action::kPaste);
    AddShortcut_("Ctrl-x",       Action::kCut);
    AddShortcut_("Ctrl-z",       Action::kUndo);
    AddShortcut_("Delete",       Action::kDelete);
    AddShortcut_("F1",           Action::kOpenHelpPanel);
    AddShortcut_("Shift-/",      Action::kOpenHelpPanel);
    AddShortcut_("Shift-Ctrl-a", Action::kSelectNone);
    AddShortcut_("Shift-Ctrl-e", Action::kToggleEdgeTarget);
    AddShortcut_("Shift-Ctrl-h", Action::kShowAll);
    AddShortcut_("Shift-Ctrl-i", Action::kOpenInfoPanel);
    AddShortcut_("Shift-Ctrl-p", Action::kTogglePointTarget);
    AddShortcut_("Shift-Ctrl-v", Action::kPasteInto);
    AddShortcut_("Shift-Ctrl-z", Action::kRedo);
    AddShortcut_("Shift-[",      Action::kMovePrevious);
    AddShortcut_("Shift-]",      Action::kMoveNext);
    AddShortcut_("[",            Action::kSwitchToPreviousTool);
    AddShortcut_("]",            Action::kSwitchToNextTool);
    AddShortcut_("x",            Action::kIncreasePrecision);
    AddShortcut_("z",            Action::kDecreasePrecision);

    // Controller buttons.
    AddShortcut_("L:Menu",       Action::kToggleLeftRadialMenu);
    AddShortcut_("R:Menu",       Action::kToggleRightRadialMenu);
    AddShortcut_("L:Center",     Action::kToggleSpecializedTool);
    AddShortcut_("R:Center",     Action::kToggleSpecializedTool);
    AddShortcut_("L:Up",         Action::kIncreasePrecision);
    AddShortcut_("R:Up",         Action::kIncreasePrecision);
    AddShortcut_("L:Down",       Action::kDecreasePrecision);
    AddShortcut_("R:Down",       Action::kDecreasePrecision);
    AddShortcut_("L:Left",       Action::kSwitchToPreviousTool);
    AddShortcut_("R:Left",       Action::kSwitchToPreviousTool);
    AddShortcut_("L:Right",      Action::kSwitchToNextTool);
    AddShortcut_("R:Right",      Action::kSwitchToNextTool);

#if ! RELEASE_BUILD
    AddShortcut_("Alt-r",        Action::kReloadScene);
#endif
}

bool ShortcutHandler::AddCustomShortcutsFromFile(const FilePath &path,
                                                 std::string &error) {
    // See if there is a "shortcuts.txt" file in the current directory. If not,
    // there is nothing to add and no error.
    if (path.Exists() && ! path.IsDirectory()) {
        std::string contents;
        if (! Util::ReadFile(path, contents)) {
            error = "Unable to read from " + path.ToString();
            return false;
        }
        return AddCustomShortcutsFromString(contents, error);
    }
    return true;
}

bool ShortcutHandler::AddCustomShortcutsFromString(const std::string &contents,
                                                   std::string &error) {
    Parser_ parser;
    return parser.AddShortcuts(contents, action_map_, error);
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
