#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// SettingsPanel is a derived Panel class that implements settings management.
class SettingsPanel : public Panel {
  public:
    /// Redefines this to set up the FilePanel.
    virtual void InitReplacementPanel(Panel &new_panel) override;

  protected:
    SettingsPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;

    /// Saves the name of the button that opened a FilePanel so it can be
    /// initialized properly.
    std::string file_panel_target_;

    /// Opens a FilePanel to get the named item
    void OpenFilePanel_(const std::string &name);

    /// Updates the settings and closes the panel.
    void AcceptSettings_();
};

typedef std::shared_ptr<SettingsPanel> SettingsPanelPtr;
