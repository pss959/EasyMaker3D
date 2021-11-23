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

    /// Opens a FileBrowserPanel to get the named item
    void OpenFileBrowser_(const std::string &name);

    /// Updates the settings and closes the panel.
    void AcceptSettings_();
};

typedef std::shared_ptr<SettingsPanel> SettingsPanelPtr;
