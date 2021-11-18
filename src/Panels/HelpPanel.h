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
    friend class Parser::Registry;

    void OpenUserGuide_();
    void OpenCheatSheet_();
    void Close_();
};

typedef std::shared_ptr<HelpPanel> HelpPanelPtr;
