#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "Base/Memory.h"
#include "Enums/Action.h"
#include "Handlers/Handler.h"

DECL_SHARED_PTR(ShortcutHandler);

/// ShortcutHandler is a derived Handler that handles keyboard shortcuts.
///
/// \ingroup Handlers
class ShortcutHandler : public Handler {
  public:
    /// Typedef for function used to apply a shortcut action.
    typedef std::function<void(Action)> ApplyActionFunc;

    ShortcutHandler();

    /// Sets the function used to apply actions.
    void SetApplyActionFunc(const ApplyActionFunc &func) {
        apply_action_func_ = func;
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
    ApplyActionFunc apply_action_func_;

    /// Maps event key strings to Action enum values.
    std::unordered_map<std::string, Action> action_map_;

    /// Handles a string representing a potential keyboard key or controller
    /// button shortcut.
    bool HandleShortcutString_(const std::string &str);
};
