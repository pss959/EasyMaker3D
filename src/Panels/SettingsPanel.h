#pragma once

#include <string>

#include "Panels/Panel.h"
#include "Util/FilePath.h"

DECL_SHARED_PTR(DropdownPane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextInputPane);
DECL_SHARED_PTR(FilePanel);
DECL_SHARED_PTR(SettingsPanel);

namespace Parser { class Registry; }

/// SettingsPanel is a derived Panel class that implements settings management.
class SettingsPanel : public Panel {
  protected:
    SettingsPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;

    TextInputPanePtr session_pane_;
    TextInputPanePtr export_pane_;
    TextInputPanePtr import_pane_;
    DropdownPanePtr  export_from_pane_;
    DropdownPanePtr  export_to_pane_;
    DropdownPanePtr  import_from_pane_;
    DropdownPanePtr  import_to_pane_;
    SliderPanePtr    tooltip_delay_slider_pane_;
    TextInputPanePtr build_volume_panes_[3];

    /// Saves the name of the button that opened a FilePanel so it can be
    /// initialized properly.
    std::string file_panel_target_;

    void InitDirectories_();
    void InitTooltipSlider_();
    void InitBuildVolume_();
    void InitConversion_();

    /// Opens a FilePanel to get the named path item.
    void OpenFilePanel_(const std::string &item_name);

    /// Initializes a FilePanel to select a path for the named item.
    void InitFilePanel_(FilePanel &file_panel, const std::string &item_name);

    /// Accepts an item by the user in the FilePanel.
    void AcceptFileItem_(const std::string &item_name, const FilePath &path);

    /// Opens a RadialMenuPanel.
    void OpenRadialMenuPanel_();

    /// Updates the settings and closes the panel.
    void AcceptSettings_();
};
