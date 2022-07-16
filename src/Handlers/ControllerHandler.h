#pragma once

#include "Base/Memory.h"
#include "Handlers/Handler.h"

DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(ControllerHandler);
DECL_SHARED_PTR(RadialMenu);

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

    /// Sets the RadialMenu objects to update.
    void SetRadialMenus(const RadialMenuPtr &l_radial_menu,
                        const RadialMenuPtr &r_radial_menu) {
        l_radial_menu_ = l_radial_menu;
        r_radial_menu_ = r_radial_menu;
    }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    ControllerPtr l_controller_;   ///< The left Controller object.
    ControllerPtr r_controller_;   ///< The right Controller object.
    RadialMenuPtr l_radial_menu_;  ///< The left RadialMenu object.
    RadialMenuPtr r_radial_menu_;  ///< The right RadialMenu object.

    void UpdateController_(const Event &event);
    void UpdateRadialMenu_(const Event &event);
};
