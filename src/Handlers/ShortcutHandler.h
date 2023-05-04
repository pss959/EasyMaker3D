#pragma once

#include <string>
#include <unordered_map>

#include "Base/Memory.h"
#include "Enums/Action.h"
#include "Handlers/Handler.h"

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
    bool AddCustomShortcutsFromFile(const FilePath &path, std::string &error);

    /// Adds custom shortcuts from the given string. Sets the error
    /// string and returns false on error. This is provided separately to make
    /// testing easier.
    bool AddCustomShortcutsFromString(const std::string &contents,
                                      std::string &error);

    /// Sets the ActionAgent used to apply actions.
    void SetActionAgent(const ActionAgentPtr &action_agent) {
        action_agent_ = action_agent;
    }

    /// Sets the string parameters to the keyboard and controller shortcut(s)
    /// for the given Action or to an empty string if there are none. If there
    /// are multiple shortcuts of either type, they are set as one
    /// comma-separated string. This is used to create the cheat sheet.
    void GetShortcutStrings(Action action,
                            std::string &keyboard_string,
                            std::string &controller_string) const;

    /// Enables or disables application shortcuts. This has no effect on
    /// debugging shortcuts so they are always available. (As opposed to
    /// calling SetEnabled(false), which would disable all shortcuts.)
    void SetAppShortcutsEnabled(bool enabled) {
        are_app_shortcuts_enabled = enabled;
    }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    class Parser_;

    typedef std::unordered_map<std::string, Action> ActionMap_;

    ActionAgentPtr action_agent_;

    /// Maps event key strings to Action enum values.
    ActionMap_ action_map_;

    /// Indicates whether application shortcuts are enabled.
    bool are_app_shortcuts_enabled = true;

    /// Handles a string representing a potential keyboard key or controller
    /// button shortcut.
    bool HandleShortcutString_(const std::string &str);

    /// Processes custom shortcuts from the contents of the shortcuts.txt file.
    void ProcessCustomShortcuts_(const std::string &contents);

    /// Adds a shortcut.
    void AddShortcut_(const std::string &key, Action action) {
        action_map_[key] = action;
    };
};
