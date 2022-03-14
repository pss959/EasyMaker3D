#pragma once

#include <string>

#include "Panels/Panel.h"

class RadialMenuInfo;
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

    void InitControllerPane_(const std::string &name,
                             const RadialMenuInfo &info);
    void AcceptEdits_();
};

typedef std::shared_ptr<RadialMenuPanel> RadialMenuPanelPtr;
