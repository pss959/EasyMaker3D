#include "Agents/SettingsAgent.h"
#include "Items/Settings.h"
#include "Panels/SettingsPanel.h"
#include "Panes/TextInputPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class SettingsPanelTest : public PanelTestBase {
  protected:
    SettingsPanelPtr panel;
    SettingsPanelTest() {
        panel = InitPanel<SettingsPanel>("SettingsPanel");
    }
};

TEST_F(SettingsPanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(SettingsPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(SettingsPanelTest, Change) {
    auto agent = GetContext().settings_agent;

    // Use default settings.
    const SettingsPtr def_settings = Settings::CreateDefault();
    agent->SetSettings(*def_settings);

    panel->SetStatus(Panel::Status::kVisible);

    // Switch to an invalid directory.
    EXPECT_FALSE(IsButtonPaneEnabled("DefaultSessionDir"));
    EXPECT_FALSE(IsButtonPaneEnabled("CurrentSessionDir"));
    auto input = SetTextInput("SessionDir", "/bad/dir");
    EXPECT_EQ("/bad/dir", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(IsButtonPaneEnabled("DefaultSessionDir"));
    EXPECT_TRUE(IsButtonPaneEnabled("CurrentSessionDir"));

    // Restore to current value (same as default).
    ClickButtonPane("CurrentSessionDir");
    EXPECT_EQ(def_settings->GetSessionDirectory(), input->GetText());
    EXPECT_FALSE(IsButtonPaneEnabled("DefaultSessionDir"));
    EXPECT_FALSE(IsButtonPaneEnabled("CurrentSessionDir"));

    // Repeat using Default button.
    input = SetTextInput("ExportDir", "/another/bad/dir");
    EXPECT_EQ("/another/bad/dir", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(IsButtonPaneEnabled("DefaultExportDir"));
    EXPECT_TRUE(IsButtonPaneEnabled("CurrentExportDir"));
    ClickButtonPane("DefaultExportDir");
    EXPECT_EQ(def_settings->GetExportDirectory(), input->GetText());
    EXPECT_FALSE(IsButtonPaneEnabled("DefaultExportDir"));
    EXPECT_FALSE(IsButtonPaneEnabled("CurrentExportDir"));


    // Try an invalid build volume size.
    input = SetTextInput("BuildVolumeDepth", "xx");
    EXPECT_FALSE(input->IsTextValid());
    input = SetTextInput("BuildVolumeDepth", "21");
    EXPECT_TRUE(input->IsTextValid());

    // Change a dropdown and build volume size and accept changes.
    auto dd = ChangeDropdownChoice("ExportTo", "Feet");
    EXPECT_TRUE(IsButtonPaneEnabled("DefaultExportConversion"));
    EXPECT_TRUE(IsButtonPaneEnabled("CurrentExportConversion"));
    input = SetTextInput("BuildVolumeDepth", "13");
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(
        UnitConversion::Units::kFeet,
        agent->GetSettings().GetExportUnitsConversion().GetToUnits());
    EXPECT_EQ(13, agent->GetSettings().GetBuildVolumeSize()[1]);
}

TEST_F(SettingsPanelTest, OpenFilePanel) {
    auto agent = GetContext().settings_agent;

    // Use default settings.
    const SettingsPtr def_settings = Settings::CreateDefault();
    agent->SetSettings(*def_settings);

    panel->SetStatus(Panel::Status::kVisible);

    ClickButtonPane("ChooseImportDir");

    // The SettingsPanel should be hidden and the FilePanel should be shown.
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, file_panel->GetStatus());

    // Change the path to the parent directory and Accept the FilePanel. This
    // should close it with result "Accept" and reopen the SettingsPanel.
    ClickButtonPane("Up");
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, file_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Clicking the Accept button in the SettingsPanel should change the stored
    // settings.
    ClickButtonPane("Accept");
    auto imp_path = def_settings->GetImportDirectory();
    EXPECT_EQ(imp_path.GetParentDirectory().ToString() + "/",
              agent->GetSettings().GetImportDirectory().ToString());
}

TEST_F(SettingsPanelTest, OpenRadialMenuPanel) {
    auto agent = GetContext().settings_agent;

    // Use default settings.
    const SettingsPtr def_settings = Settings::CreateDefault();
    agent->SetSettings(*def_settings);

    panel->SetStatus(Panel::Status::kVisible);

    ClickButtonPane("EditRadialMenus");

    // The SettingsPanel should be hidden and the RadialMenuPanel should be
    // shown.
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto menu_panel = GetCurrentPanel();
    EXPECT_EQ("RadialMenuPanel", menu_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, menu_panel->GetStatus());

    // Change the mode and Accept the RadialMenuPanel. This should close it
    // with result "Accept" and reopen the SettingsPanel.
    ActivateRadioButtonPane("Mode3");
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, menu_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Clicking the Accept button in the SettingsPanel should change the stored
    // settings.
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(RadialMenusMode::kIndependent,
                   agent->GetSettings().GetRadialMenusMode());
}
