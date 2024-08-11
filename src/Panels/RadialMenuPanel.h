//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/Hand.h"
#include "Panels/Panel.h"

DECL_SHARED_PTR(BoxPane);
DECL_SHARED_PTR(RadialMenu);
DECL_SHARED_PTR(RadialMenuInfo);
DECL_SHARED_PTR(RadialMenuPanel);

namespace Parser { class Registry; }

/// RadialMenuPanel is a derived Panel class that allows the user to edit the
/// actions attached to buttons in radial menus.
///
/// \ingroup Panels
class RadialMenuPanel : public Panel {
  protected:
    RadialMenuPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    size_t            mode_index_;
    RadialMenuPtr     left_menu_, right_menu_;
    RadialMenuInfoPtr left_info_, right_info_;

    BoxPane & GetControllerPane_(Hand hand);
    RadialMenuPtr InitControllerPane_(Hand hand);
    void UpdateControllerPane_(Hand hand, const RadialMenuInfo &info);
    void ModeChanged_(size_t index);
    void CountChanged_(Hand hand, size_t index);
    void ButtonClicked_(Hand hand, size_t index);
    void AcceptEdits_();

    friend class Parser::Registry;
};
