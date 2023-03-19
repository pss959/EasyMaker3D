#pragma once

#include <string>
#include <unordered_map>

#include "Base/Memory.h"
#include "Enums/Action.h"
#include "Handlers/Handler.h"

DECL_SHARED_PTR(ActionAgent);
DECL_SHARED_PTR(ShortcutHandler);

/// ShortcutHandler is a derived Handler that handles keyboard shortcuts.
///
/// \ingroup Handlers
class ShortcutHandler : public Handler {
  public:
    ShortcutHandler();

    /// Adds custom shortcuts from the "shortcuts.txt" file in the current
    /// directory. Sets the error string and returns false on error.
    bool AddCustomShortcuts(std::string &error);

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

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    ActionAgentPtr action_agent_;

    /// Maps event key strings to Action enum values.
    std::unordered_map<std::string, Action> action_map_;

    /// Handles a string representing a potential keyboard key or controller
    /// button shortcut.
    bool HandleShortcutString_(const std::string &str);
};
