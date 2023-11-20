#pragma once

#include <unordered_map>

#include "Enums/Action.h"
#include "Handlers/Handler.h"
#include "Util/Memory.h"

class FilePath;
DECL_SHARED_PTR(ActionAgent);
DECL_SHARED_PTR(ShortcutHandler);

/// ShortcutHandler is a derived Handler that handles keyboard shortcuts.
///
/// \ingroup Handlers
class ShortcutHandler : public Handler {
  public:
    ShortcutHandler();

    /// Adds custom shortcuts from the given FilePath. Sets the error string
    /// and returns false on error.
    bool AddCustomShortcutsFromFile(const FilePath &path, Str &error);

    /// Adds custom shortcuts from the given string. Sets the error
    /// string and returns false on error. This is provided separately to make
    /// testing easier.
    bool AddCustomShortcutsFromString(const Str &contents, Str &error);

    /// Sets the ActionAgent used to apply actions.
    void SetActionAgent(const ActionAgentPtr &action_agent) {
        action_agent_ = action_agent;
    }

    /// Sets the string parameters to the keyboard and controller shortcut(s)
    /// for the given Action or to an empty string if there are none. If there
    /// are multiple shortcuts of either type, they are set as one
    /// comma-separated string. This is used to create the cheat sheet.
    void GetShortcutStrings(Action action, Str &keyboard_string,
                            Str &controller_string) const;

    /// Enables or disables application shortcuts.
    void SetAppShortcutsEnabled(bool enabled) {
        are_app_shortcuts_enabled = enabled;
    }

    /// Enables or disables debugging shortcuts. (Has effect only when
    /// ENABLE_DEBUG_FEATURES is defined as true.)
    void SetDebugShortcutsEnabled(bool enabled) {
        are_debug_shortcuts_enabled = enabled;
    }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    class Parser_;

    using ActionMap_ = std::unordered_map<Str, Action>;

    ActionAgentPtr action_agent_;

    /// Maps event key strings to Action enum values.
    ActionMap_ action_map_;

    /// Indicates whether application shortcuts are enabled.
    bool are_app_shortcuts_enabled = true;

    /// Indicates whether debug shortcuts are enabled.
    bool are_debug_shortcuts_enabled = true;

    /// Handles a string representing a potential keyboard key or controller
    /// button shortcut.
    bool HandleShortcutString_(const Str &str);

    /// Processes custom shortcuts from the contents of the shortcuts.txt file.
    void ProcessCustomShortcuts_(const Str &contents);

    /// Adds a shortcut.
    void AddShortcut_(const Str &key, Action action) {
        action_map_[key] = action;
    };
};
