#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// ActionPanel is a derived Panel class that allows the user to choose an
/// action to attach to a radial menu button.
class ActionPanel : public Panel {
  protected:
    ActionPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    void Accept_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<ActionPanel> ActionPanelPtr;
