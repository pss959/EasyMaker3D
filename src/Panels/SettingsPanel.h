#pragma once

#include <string>

#include "Panels/Panel.h"
#include "Util/FilePath.h"

class FilePanel;

namespace Parser { class Registry; }

/// SettingsPanel is a derived Panel class that implements settings management.
class SettingsPanel : public Panel {
  protected:
    SettingsPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;

    /// Saves the name of the button that opened a FilePanel so it can be
    /// initialized properly.
    std::string file_panel_target_;

    /// Opens a FilePanel to get the named path item.
    void OpenFilePanel_(const std::string &item_name);

    /// Initializes a FilePanel to select a path for the named item.
    void InitFilePanel_(FilePanel &file_panel, const std::string &item_name);

    /// Accepts an item by the user in the FilePanel.
    void AcceptFileItem_(const std::string &item_name, const FilePath &path);

    /// Updates the settings and closes the panel.
    void AcceptSettings_();
};

typedef std::shared_ptr<SettingsPanel> SettingsPanelPtr;
