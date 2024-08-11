//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_TRUE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
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
    EXPECT_FALSE(pi.IsButtonPaneEnabled("DefaultSessionDir"));
    EXPECT_FALSE(pi.IsButtonPaneEnabled("CurrentSessionDir"));
    auto input = pi.SetTextInput("SessionDir", "/bad/dir");
    EXPECT_EQ("/bad/dir", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(pi.IsButtonPaneEnabled("DefaultSessionDir"));
    EXPECT_TRUE(pi.IsButtonPaneEnabled("CurrentSessionDir"));

    // Restore to current value (same as default).
    pi.ClickButtonPane("CurrentSessionDir");
    EXPECT_EQ(def_settings->GetSessionDirectory(), input->GetText());
    EXPECT_FALSE(pi.IsButtonPaneEnabled("DefaultSessionDir"));
    EXPECT_FALSE(pi.IsButtonPaneEnabled("CurrentSessionDir"));

    // Repeat using Default button.
    input = pi.SetTextInput("ExportDir", "/another/bad/dir");
    EXPECT_EQ("/another/bad/dir", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(pi.IsButtonPaneEnabled("DefaultExportDir"));
    EXPECT_TRUE(pi.IsButtonPaneEnabled("CurrentExportDir"));
    pi.ClickButtonPane("DefaultExportDir");
    EXPECT_EQ(def_settings->GetExportDirectory(), input->GetText());
    EXPECT_FALSE(pi.IsButtonPaneEnabled("DefaultExportDir"));
    EXPECT_FALSE(pi.IsButtonPaneEnabled("CurrentExportDir"));

    // Try an invalid build volume size.
    input = pi.SetTextInput("BuildVolumeDepth", "xx");
    EXPECT_FALSE(input->IsTextValid());
    input = pi.SetTextInput("BuildVolumeDepth", "21");
    EXPECT_TRUE(input->IsTextValid());

    // Change a dropdown and build volume size and accept changes.
    auto dd = pi.ChangeDropdownChoice("ExportTo", "Feet");
    EXPECT_TRUE(pi.IsButtonPaneEnabled("DefaultExportConversion"));
    EXPECT_TRUE(pi.IsButtonPaneEnabled("CurrentExportConversion"));
    input = pi.SetTextInput("BuildVolumeDepth", "13");
    pi.ClickButtonPane("Accept");
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

    pi.ClickButtonPane("ChooseImportDir");

    // The SettingsPanel should be hidden and the FilePanel should be shown.
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, file_panel->GetStatus());

    // Change the path to the parent directory and Accept the FilePanel. This
    // should close it with result "Accept" and reopen the SettingsPanel.
    pi.ClickButtonPane("Up");
    pi.ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, file_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Clicking the Accept button in the SettingsPanel should change the stored
    // settings.
    pi.ClickButtonPane("Accept");
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

    pi.ClickButtonPane("EditRadialMenus");

    // The SettingsPanel should be hidden and the RadialMenuPanel should be
    // shown.
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto menu_panel = GetCurrentPanel();
    EXPECT_EQ("RadialMenuPanel", menu_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, menu_panel->GetStatus());

    // Change the mode and Accept the RadialMenuPanel. This should close it
    // with result "Accept" and reopen the SettingsPanel.
    pi.ActivateRadioButtonPane("Mode3");
    pi.ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, menu_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Clicking the Accept button in the SettingsPanel should change the stored
    // settings.
    pi.ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(RadialMenusMode::kIndependent,
                   agent->GetSettings().GetRadialMenusMode());
}
