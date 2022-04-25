#pragma once

#include "Handlers/Handler.h"
#include "Memory.h"

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(ControllerHandler);

/// The ControllerHandler class manages updating the controllers from input
/// events.
///
/// \ingroup Handlers
class ControllerHandler : public Handler {
  public:
    /// Sets the Controller objects to update.
    void SetControllers(const ControllerPtr &l_controller,
                        const ControllerPtr &r_controller) {
        l_controller_ = l_controller;
        r_controller_ = r_controller;
    }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    ControllerPtr l_controller_;  ///< The left Controller object.
    ControllerPtr r_controller_;  ///< The right Controller object.
};
