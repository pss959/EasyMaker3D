#pragma once

#include <string>
#include <unordered_map>

#include "Base/Memory.h"
#include "Enums/Action.h"
#include "Handlers/Handler.h"

DECL_SHARED_PTR(ShortcutHandler);
DECL_SHARED_PTR(ActionManager);

/// ShortcutHandler is a derived Handler that handles keyboard shortcuts.
///
/// \ingroup Handlers
class ShortcutHandler : public Handler {
  public:
    ShortcutHandler();

    /// Sets the ActionManager used to apply actions.
    void SetActionManager(const ActionManagerPtr &action_manager) {
        action_manager_ = action_manager;
    }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    ActionManagerPtr action_manager_;

    /// Maps event key strings to Action enum values.
    std::unordered_map<std::string, Action> action_map_;

    /// Handles a string representing a potential keyboard key or controller
    /// button shortcut.
    bool HandleShortcutString_(const std::string &str);
};
