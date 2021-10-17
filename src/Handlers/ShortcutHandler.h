#pragma once

#include <vector>

#include "Handlers/Handler.h"
#include "Managers/ActionManager.h"

/// ShortcutHandler is a derived Handler that handles keyboard shortcuts.
/// \ingroup Handlers
class ShortcutHandler : public Handler {
  public:
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
};
