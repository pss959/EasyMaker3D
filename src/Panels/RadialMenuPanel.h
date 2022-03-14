#pragma once

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// RadialMenuPanel is a derived Panel class that allows the user to edit the
/// actions attached to buttons in radial menus.
class RadialMenuPanel : public Panel {
  protected:
    RadialMenuPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;

    void AcceptEdits_();
};

typedef std::shared_ptr<RadialMenuPanel> RadialMenuPanelPtr;
