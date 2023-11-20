#pragma once

#include "Panels/Panel.h"
#include "Util/FilePath.h"

DECL_SHARED_PTR(DropdownPane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextInputPane);
DECL_SHARED_PTR(FilePanel);
DECL_SHARED_PTR(SettingsPanel);

namespace Parser { class Registry; }

/// SettingsPanel is a derived Panel class that implements settings management.
///
/// \ingroup Panels
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

    /// Stores a default Settings for comparison.
    SettingsPtr default_settings_;

    /// Saves the name of the button that opened a FilePanel so it can be
    /// initialized properly.
    Str file_panel_target_;

    /// This is set to true so that changes to Pane values do not cause the
    /// EnableDefaultAndCurrentButtons_() function to change anything.
    bool ignore_button_updates_ = false;

    void InitDirectories_();
    void InitTooltipSlider_();
    void InitBuildVolume_();
    void InitConversion_();

    /// Sets the named item to its default value.
    void SetToDefault_(const Str &name);

    /// Sets the named item to its current value.
    void SetToCurrent_(const Str &name);

    /// Updates the enable status of the Default and Current buttons.
    void EnableDefaultAndCurrentButtons_();

    /// Sets the named item to the value in the given Settings instance.
    void UpdateFromSettings_(const Settings &settings, const Str &name);

    /// Opens a FilePanel to get the named path item.
    void OpenFilePanel_(const Str &item_name);

    /// Initializes a FilePanel to select a path for the named item.
    void InitFilePanel_(FilePanel &file_panel, const Str &item_name);

    /// Accepts an item by the user in the FilePanel.
    void AcceptFileItem_(const Str &item_name, const FilePath &path);

    /// Opens a RadialMenuPanel.
    void OpenRadialMenuPanel_();

    /// Updates the settings and closes the panel.
    void AcceptSettings_();

    /// Parses a size value from a string. Returns false on error.
    static bool ParseSize_(const Str &s, float &size);
};
