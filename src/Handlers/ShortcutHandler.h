#pragma once

#include <vector>

#include "Interfaces/IApplication.h"
#include "Interfaces/IHandler.h"

//! ShortcutHandler is a derived IHandler that handles keyboard shortcuts.
class ShortcutHandler : public IHandler {
  public:
    ShortcutHandler(IApplication &app);
    virtual ~ShortcutHandler();

    virtual const char * GetClassName() const override {
        return "ShortcutHandler";
    }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    IApplication &app_;
};
