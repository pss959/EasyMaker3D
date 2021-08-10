#pragma once

#include <vector>

#include "Interfaces/IApplication.h"
#include "Interfaces/IHandler.h"

//! ShortcutHandler is a derived IHandler that handles keyboard shortcuts.
class ShortcutHandler : public IHandler {
  public:
    //! The constructor is passed an IApplication::Context which contains
    //! objects the ShortcutHandler needs for processing.
    ShortcutHandler(const IApplication::Context &app_context);
    virtual ~ShortcutHandler();

    virtual const char * GetClassName() const override {
        return "ShortcutHandler";
    }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    const IApplication::Context &app_context_;
};
