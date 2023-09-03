#include "Base/Event.h"
#include "Enums/FileFormat.h"
#include "Panels/FilePanel.h"
#include "Panes/TextInputPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Util/FakeFileSystem.h"

/// \ingroup Tests
class FilePanelTest : public PanelTestBase {
  protected:
    FilePanelPtr panel;
    FilePanelTest() {
        panel = InitPanel<FilePanel>("FilePanel");

        // Set up some directories in the FakeFileSystem to make the direction
        // button enabling consistent.
        UseRealFileSystem(false);
        auto fs = GetFakeFileSystem();
        fs->AddDir("/a");
        fs->AddDir("/a/b");
        fs->AddDir("/a/b/c");
        fs->AddFile("/a/b/c/d.jpg");
        fs->AddFile("/a/b/c/d.stl");
    }
    ~FilePanelTest() {}
};

TEST_F(FilePanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(FilePanelTest, Show) {
    panel->SetTitle("A Title");
    panel->SetTargetType(FilePanel::TargetType::kDirectory);
    panel->SetInitialPath("/a/b");
    panel->SetFileFormats(std::vector<FileFormat>{
            FileFormat::kTextSTL, FileFormat::kBinarySTL });
    panel->SetExtension(".stl");
    panel->SetHighlightPath("/a/b/c", "ANNOTATION");

    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Resize to make sure focus stays put.
    const auto fp = panel->GetFocusedPane();
    EXPECT_NOT_NULL(fp);
    panel->SetSize(panel->GetSize() + Vector2f(10, 0));
    EXPECT_EQ(fp, panel->GetFocusedPane());
}

TEST_F(FilePanelTest, Scroll) {
    panel->SetInitialPath("/a/b/c");
    panel->SetStatus(Panel::Status::kVisible);

    Event event;
    event.device = Event::Device::kMouse;
    EXPECT_FALSE(panel->HandleEvent(event));

    event.flags.Set(Event::Flag::kPosition1D);
    event.position1D = -1;
    EXPECT_TRUE(panel->HandleEvent(event));
}

TEST_F(FilePanelTest, Directions) {
    panel->SetInitialPath("/a/b/c");
    panel->SetStatus(Panel::Status::kVisible);

    auto input = FindTypedPane<TextInputPane>("Input");

    EXPECT_EQ("/a/b/c/", input->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Up"));
    ClickButtonPane("Up");
    EXPECT_EQ("/a/b/", input->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Back"));
    ClickButtonPane("Back");
    EXPECT_EQ("/a/b/c/", input->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Forward"));
    ClickButtonPane("Forward");
    EXPECT_EQ("/a/b/", input->GetText());

    // Click on a directory buttons.
    ClickButtonPane("Dir_0");
    EXPECT_EQ("/a/b/c/", input->GetText());

    EXPECT_TRUE(IsButtonPaneEnabled("Home"));
    ClickButtonPane("Home");
    EXPECT_EQ("/home/user", input->GetText());
}

TEST_F(FilePanelTest, ChooseDirectory) {
    panel->SetTitle("Title");
    panel->SetTargetType(FilePanel::TargetType::kDirectory);
    panel->SetInitialPath("/a/b/c");
    panel->SetHighlightPath("/a/b/c", "ANNOTATION");

    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Change the path to the parent directory and Accept the FilePanel.
    ClickButtonPane("Up");
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());

    // Test the results.
    EXPECT_EQ("/a/b", panel->GetPath().ToString());
}

TEST_F(FilePanelTest, ChooseNewFile) {
    panel->SetTitle("Title");
    panel->SetTargetType(FilePanel::TargetType::kNewFile);
    panel->SetInitialPath("/a/b/c");
    panel->SetFileFormats(std::vector<FileFormat>{
            FileFormat::kTextSTL, FileFormat::kBinarySTL });
    panel->SetExtension(".stl");
    panel->SetHighlightPath("/a/b/c", "ANNOTATION");

    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Change the format dropdown.
    ChangeDropdownChoice("Format", "Binary STL");

    // Cannot accept a directory.
    EXPECT_FALSE(IsButtonPaneEnabled("Accept"));

    // Enter a file name with the wrong extension. Should be acceptable.
    SetTextInput("Input", "/a/b/c/test.jpg");
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Enter a file name with the correct extension. Should be acceptable.
    SetTextInput("Input", "/a/b/c/test.stl");
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Enter a file name with no extension. Should be acceptable.
    SetTextInput("Input", "/a/b/c/test");
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Accept it and test the results.
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept",                   GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("/a/b/c/test.stl",          panel->GetPath().ToString());
    EXPECT_EQ(FileFormat::kBinarySTL,     panel->GetFileFormat());
}

TEST_F(FilePanelTest, ChooseExistingFile) {
    panel->SetTitle("Title");
    panel->SetTargetType(FilePanel::TargetType::kExistingFile);
    panel->SetInitialPath("/a/b/c");
    panel->SetFileFormats(std::vector<FileFormat>{
            FileFormat::kTextSTL, FileFormat::kBinarySTL });
    panel->SetExtension(".stl");
    panel->SetHighlightPath("/a/b/c", "ANNOTATION");

    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Cannot accept a directory.
    EXPECT_FALSE(IsButtonPaneEnabled("Accept"));

    // Enter a file name with no extension. Should not be acceptable.
    SetTextInput("Input", "/a/b/c/test");
    EXPECT_FALSE(IsButtonPaneEnabled("Accept"));

    // Enter an existing file name with the wrong extension. Should be
    // acceptable.
    SetTextInput("Input", "/a/b/c/d.jpg");
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Enter an existing file name with the correct extension. Should be
    // acceptable.
    SetTextInput("Input", "/a/b/c/d.stl");
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Accept it and test the results.
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept",                   GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("/a/b/c/d.stl",             panel->GetPath().ToString());
    EXPECT_EQ(FileFormat::kTextSTL,       panel->GetFileFormat());
}

TEST_F(FilePanelTest, OverwriteFile) {
    panel->SetTitle("Title");
    panel->SetTargetType(FilePanel::TargetType::kNewFile);
    panel->SetInitialPath("/a/b/c");
    panel->SetHighlightPath("/a/b/c", "ANNOTATION");
    panel->SetStatus(Panel::Status::kVisible);

    // Enter an existing file name.
    SetTextInput("Input", "/a/b/c/d.stl");

    // Accept it. This should bring up a DialogPanel asking whether to
    // overwrite the file.
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());

    // Click "No" in the dialog -> should restore the FilePanel.
    ClickButtonPane("Button0");  // "No"
    EXPECT_EQ("No", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Accept again and this time click "Yes" in the dialog -> should close the
    // DialogPanel and FilePanel.
#if XXXX
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());
    ClickButtonPane("Button1");  // "Yes"
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_EQ("/a/b/c/d.stl", panel->GetPath().ToString());
#endif
}
