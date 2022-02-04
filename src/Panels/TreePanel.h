#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// TreePanel is a derived Panel class that is attached to he WallBoard. It
/// displays the current session info, current Models, and allows various
/// interactions with them.
class TreePanel : public Panel {
  protected:
    TreePanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<TreePanel> TreePanelPtr;
