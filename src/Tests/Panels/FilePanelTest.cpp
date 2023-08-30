#include "Enums/FileFormat.h"
#include "Panels/FilePanel.h"
#include "Panes/TextInputPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class FilePanelTest : public PanelTestBase {
  protected:
    FilePanelPtr panel;
    explicit FilePanelTest(bool need_text = false) :
        PanelTestBase(need_text) {
        panel = InitPanel<FilePanel>("FilePanel");
    }
};

/// \ingroup Tests
class FilePanelTestWithText : public FilePanelTest {
  protected:
    FilePanelTestWithText() : FilePanelTest(true) {}
};

TEST_F(FilePanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(FilePanelTest, Show) {
    panel->SetTitle("A Title");
    panel->SetTargetType(FilePanel::TargetType::kDirectory);
    panel->SetInitialPath("/a/b/c");
    panel->SetFileFormats(std::vector<FileFormat>{
            FileFormat::kTextSTL, FileFormat::kBinarySTL });
    panel->SetExtension(".stl");
    panel->SetHighlightPath("/a/b/c", "ANNOTATION");

    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(FilePanelTest, Directions) {
    panel->SetInitialPath("/a/b/c");
    panel->SetIsShown(true);

    ClickButtonPane("Up");
    EXPECT_EQ("/a/b", panel->GetPath().ToString());
    ClickButtonPane("Back");
    EXPECT_EQ("/a/b/c", panel->GetPath().ToString());
    ClickButtonPane("Forward");
    EXPECT_EQ("/a/b", panel->GetPath().ToString());
    ClickButtonPane("Home");
    EXPECT_EQ("", panel->GetPath().ToString());

}

#if XXXX
TEST_F(FilePanelTestWithText, Change) {
    auto agent = GetContext().file_agent;

    // Use default file.
    const FilePtr def_file = File::CreateDefault();
    agent->SetFile(*def_file);

    panel->SetIsShown(true);

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
    EXPECT_EQ(def_file->GetSessionDirectory(), input->GetText());
    EXPECT_FALSE(IsButtonPaneEnabled("DefaultSessionDir"));
    EXPECT_FALSE(IsButtonPaneEnabled("CurrentSessionDir"));

    // Repeat using Default button.
    input = SetTextInput("ExportDir", "/another/bad/dir");
    EXPECT_EQ("/another/bad/dir", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(IsButtonPaneEnabled("DefaultExportDir"));
    EXPECT_TRUE(IsButtonPaneEnabled("CurrentExportDir"));
    ClickButtonPane("DefaultExportDir");
    EXPECT_EQ(def_file->GetExportDirectory(), input->GetText());
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
        agent->GetFile().GetExportUnitsConversion().GetToUnits());
    EXPECT_EQ(13, agent->GetFile().GetBuildVolumeSize()[1]);
}

TEST_F(FilePanelTestWithText, OpenFilePanel) {
    auto agent = GetContext().file_agent;

    // Use default file.
    const FilePtr def_file = File::CreateDefault();
    agent->SetFile(*def_file);

    panel->SetIsShown(true);

    ClickButtonPane("ChooseImportDir");

    // The FilePanel should be hidden and the FilePanel should be shown.
    EXPECT_FALSE(panel->IsShown());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_TRUE(file_panel->IsShown());

    // Change the path to the parent directory and Accept the FilePanel. This
    // should close it with result "Accept" and reopen the FilePanel.
    ClickButtonPane("Up");
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_FALSE(file_panel->IsShown());
    EXPECT_TRUE(panel->IsShown());

    // Clicking the Accept button in the FilePanel should change the stored
    // file.
    ClickButtonPane("Accept");
    EXPECT_EQ(def_file->GetImportDirectory().GetParentDirectory(),
              agent->GetFile().GetImportDirectory());
}

TEST_F(FilePanelTestWithText, OpenRadialMenuPanel) {
    auto agent = GetContext().file_agent;

    // Use default file.
    const FilePtr def_file = File::CreateDefault();
    agent->SetFile(*def_file);

    panel->SetIsShown(true);

    ClickButtonPane("EditRadialMenus");

    // The FilePanel should be hidden and the RadialMenuPanel should be
    // shown.
    EXPECT_FALSE(panel->IsShown());
    auto menu_panel = GetCurrentPanel();
    EXPECT_EQ("RadialMenuPanel", menu_panel->GetTypeName());
    EXPECT_TRUE(menu_panel->IsShown());

    // Change the mode and Accept the RadialMenuPanel. This should close it
    // with result "Accept" and reopen the FilePanel.
    ActivateRadioButtonPane("Mode3");
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_FALSE(menu_panel->IsShown());
    EXPECT_TRUE(panel->IsShown());

    // Clicking the Accept button in the FilePanel should change the stored
    // file.
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(RadialMenusMode::kIndependent,
                   agent->GetFile().GetRadialMenusMode());
}
#endif
