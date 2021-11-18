#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// SettingsPanel is a derived Panel class that implements settings management.
class SettingsPanel : public Panel {
  protected:
    SettingsPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<SettingsPanel> SettingsPanelPtr;
