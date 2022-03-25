#pragma once

#include <string>

#include "Enums/Hand.h"
#include "Panels/Panel.h"

DECL_SHARED_PTR(BoxPane);
DECL_SHARED_PTR(RadialMenu);
DECL_SHARED_PTR(RadialMenuInfo);
DECL_SHARED_PTR(RadialMenuPanel);

namespace Parser { class Registry; }

/// RadialMenuPanel is a derived Panel class that allows the user to edit the
/// actions attached to buttons in radial menus.
class RadialMenuPanel : public Panel {
  protected:
    RadialMenuPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    RadialMenuPtr     left_menu_, right_menu_;
    RadialMenuInfoPtr left_info_, right_info_;

    BoxPane & GetControllerPane_(Hand hand);
    RadialMenuPtr InitControllerPane_(Hand hand);
    void UpdateControllerPane_(Hand hand, const RadialMenuInfo &info);
    void CountChanged_(Hand hand, size_t index);
    void ButtonClicked_(Hand hand, size_t index);
    void AcceptEdits_();

    friend class Parser::Registry;
};
