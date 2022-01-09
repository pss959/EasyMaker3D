#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// HelpPanel is a derived Panel class that implements help management.
class HelpPanel : public Panel {
  protected:
    HelpPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    void OpenUserGuide_();
    void OpenCheatSheet_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<HelpPanel> HelpPanelPtr;
