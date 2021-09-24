#pragma once

#include <vector>

#include "Interfaces/IApplication.h"
#include "Handlers/Handler.h"

/// ShortcutHandler is a derived Handler that handles keyboard shortcuts.
/// \ingroup Handlers
class ShortcutHandler : public Handler {
  public:
    ShortcutHandler(IApplication &app);
    virtual ~ShortcutHandler();

    // XXXX Temporary way to handle exit.
    bool ShouldExit() { return should_exit_; }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    IApplication &app_;
    bool should_exit_ = false; // XXXX Temporary
};
