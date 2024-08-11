//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/Event.h"
#include "Enums/FileFormat.h"
#include "Panels/FilePanel.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Util/FakeFileSystem.h"

// ----------------------------------------------------------------------------
// FilePanelTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class FilePanelTest : public PanelTestBase {
  protected:
    FilePanelPtr panel;  ///< FilePanel set up in constructor.

    FilePanelTest();
    ~FilePanelTest() {}

    /// Tests that the contents of the FilePanel contains only file buttons
    /// for the named subdirectories and files.
    void TestFiles(const Str &when, const StrVec &subdirs, const StrVec &files);
};

FilePanelTest::FilePanelTest() {
    panel = InitPanel<FilePanel>("FilePanel");

    // Set up some directories in the FakeFileSystem to make the direction
    // button enabling consistent.
    UseRealFileSystem(false);
    auto fs = GetFakeFileSystem();
    fs->AddDir("/a");
    fs->AddDir("/a/b");
    fs->AddDir("/a/b/c");
    fs->AddDir("/a/b/c/subdir0");
    fs->AddDir("/a/b/c/subdir1");
    fs->AddFile("/a/b/c/file0.jpg");
    fs->AddFile("/a/b/c/file1.stl");
    fs->AddFile("/a/b/c/hidden0.txt", true);
    fs->AddFile("/a/b/c/hidden1.txt", true);
}

void FilePanelTest::TestFiles(const Str &when,
                              const StrVec &subdirs, const StrVec &files) {
    auto get_fb = [&](const Str &prefix, size_t index){
        return panel->GetPane()->FindSubPane(prefix + Util::ToString(index));
    };

    // Subdirs.
    for (size_t i = 0; i < subdirs.size(); ++i) {
        auto but = get_fb("Dir_", i);
        EXPECT_NOT_NULL(but) << " " << when << " subdir " << i;
        auto text = but->FindTypedSubPane<TextPane>("ButtonText");
        EXPECT_EQ(subdirs[i], text->GetText()) << when << " subdir " << i;
    }
    // Make sure there are no more subdirs.
    EXPECT_NULL(get_fb("Dir_", subdirs.size())) << " " << when;

    // Files
    for (size_t i = 0; i < subdirs.size(); ++i) {
        auto but = get_fb("File_", i);
        EXPECT_NOT_NULL(but) << " " << when << " file " << i;
        auto text = but->FindTypedSubPane<TextPane>("ButtonText");
        EXPECT_EQ(files[i], text->GetText()) << when << " file " << i;
    }
    // Make sure there are no more subdirs.
    EXPECT_NULL(get_fb("File_", files.size())) << " " << when;
}

// ----------------------------------------------------------------------------
// FilePanelTest tests.
// ----------------------------------------------------------------------------

TEST_F(FilePanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_TRUE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
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
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Up"));
    pi.ClickButtonPane("Up");
    EXPECT_EQ("/a/b/", input->GetText());
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Back"));
    pi.ClickButtonPane("Back");
    EXPECT_EQ("/a/b/c/", input->GetText());
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Forward"));
    pi.ClickButtonPane("Forward");
    EXPECT_EQ("/a/b/", input->GetText());

    // Click on a directory buttons.
    pi.ClickButtonPane("Dir_0");
    EXPECT_EQ("/a/b/c/", input->GetText());

    EXPECT_TRUE(pi.IsButtonPaneEnabled("Home"));
    pi.ClickButtonPane("Home");
    EXPECT_EQ("/home/user", input->GetText());
}

TEST_F(FilePanelTest, HiddenFiles) {
    panel->SetInitialPath("/a/b/c");
    panel->SetTargetType(FilePanel::TargetType::kExistingFile);
    panel->SetStatus(Panel::Status::kVisible);

    auto input = FindTypedPane<TextInputPane>("Input");

    // Verify that only non-hidden subdirs and files are visible. There should
    // be two directories and two files.
    TestFiles("Before showing hidden",
              StrVec{ "subdir0", "subdir1" },
              StrVec{ "file0.jpg", "file1.stl" });

    // Show hidden files. There should now be two more files.
    pi.ToggleCheckboxPane("HiddenFiles");
    TestFiles("After showing hidden",
              StrVec{ "subdir0", "subdir1" },
              StrVec{ "file0.jpg", "file1.stl", "hidden0.txt", "hidden1.txt" });

    // Toggle again; should make files hidden again.
    pi.ToggleCheckboxPane("HiddenFiles");
    TestFiles("After hiding again",
              StrVec{ "subdir0", "subdir1" },
              StrVec{ "file0.jpg", "file1.stl" });
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
    pi.ClickButtonPane("Up");
    pi.ClickButtonPane("Accept");
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
    pi.ChangeDropdownChoice("Format", "Binary STL");

    // Cannot accept a directory.
    EXPECT_FALSE(pi.IsButtonPaneEnabled("Accept"));

    // Enter a file name with the wrong extension. Should be acceptable.
    pi.SetTextInput("Input", "/a/b/c/test.jpg");
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));

    // Enter a file name with the correct extension. Should be acceptable.
    pi.SetTextInput("Input", "/a/b/c/test.stl");
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));

    // Enter a file name with no extension. Should be acceptable.
    pi.SetTextInput("Input", "/a/b/c/test");
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));

    // Accept it and test the results.
    pi.ClickButtonPane("Accept");
    EXPECT_EQ("Accept",                        GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("/a/b/c/test.stl",               panel->GetPath().ToString());
    EXPECT_EQ(FileFormat::kBinarySTL,          panel->GetFileFormat());
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
    EXPECT_FALSE(pi.IsButtonPaneEnabled("Accept"));

    // Click on an existing file name with the correct extension. Should be
    // acceptable. This should also focus on the file button for this file.
    pi.ClickButtonPane("File_0");
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));
    const auto but = panel->GetFocusedPane();
    EXPECT_NOT_NULL(but);
    auto text = but->FindTypedSubPane<TextPane>("ButtonText");
    EXPECT_EQ("file1.stl", text->GetText());

    // Enter a file name with no extension. Should not be acceptable.
    pi.SetTextInput("Input", "/a/b/c/test");
    EXPECT_FALSE(pi.IsButtonPaneEnabled("Accept"));

    // Enter an existing file name with the wrong extension. Should not be
    // acceptable.
    pi.SetTextInput("Input", "/a/b/c/file0.jpg");
    EXPECT_FALSE(pi.IsButtonPaneEnabled("Accept"));

    // Enter and accept a valid file name and test the results.
    pi.SetTextInput("Input", "/a/b/c/file1.stl");
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));
    pi.ClickButtonPane("Accept");
    EXPECT_EQ("Accept",                        GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("/a/b/c/file1.stl",              panel->GetPath().ToString());
    EXPECT_EQ(FileFormat::kTextSTL,            panel->GetFileFormat());
}

TEST_F(FilePanelTest, OverwriteFile) {
    panel->SetTitle("Title");
    panel->SetTargetType(FilePanel::TargetType::kNewFile);
    panel->SetInitialPath("/a/b/c");
    panel->SetHighlightPath("/a/b/c", "ANNOTATION");
    panel->SetStatus(Panel::Status::kVisible);

    // Enter an existing file name.
    pi.SetTextInput("Input", "/a/b/c/file1.stl");

    // Accept it. This should bring up a DialogPanel asking whether to
    // overwrite the file.
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));
    pi.ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());

    // Click "No" in the dialog -> should restore the FilePanel.
    pi.ClickButtonPane("Button0");  // "No"
    EXPECT_EQ("No", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Accept again and this time click "Yes" in the dialog -> should close the
    // DialogPanel and FilePanel.
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));
    pi.ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());
    pi.ClickButtonPane("Button1");  // "Yes"
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Yes", GetCloseResult());
    EXPECT_EQ("/a/b/c/file1.stl", panel->GetPath().ToString());
}
