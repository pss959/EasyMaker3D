//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Testing.h"
#include "Tests/Util/FakeFileSystem.h"
#include "Util/FilePathList.h"

/// \ingroup Tests
class FilePathListTest : public ::testing::Test {
  protected:
    /// Make the FakeFileSystem easily accessible to tests.
    std::shared_ptr<FakeFileSystem> fs;

    FilePathListTest() {
        // Use a FakeFileSystem with a simple hierarchy for testing.
        fs.reset(new FakeFileSystem);
        fs->AddDir("/abc");
        fs->AddDir("/abc/def");
        fs->AddDir("/abc/def/subdir0");
        fs->AddFile("/abc/def/file0.txt");
        fs->AddFile("/abc/def/file1.txt");
        fs->AddDir("/ghi");
        fs->AddDir("/ghi/jkl");
        fs->AddFile("/ghi/jkl/file2.txt");
        FileSystem::Install(fs);
    }
};

TEST_F(FilePathListTest, Basics) {
    FilePathList fpl;
    fpl.Init(FilePath("/abc/def"));
    EXPECT_EQ("/abc/def", fpl.GetCurrent().ToString());
    EXPECT_TRUE(fpl.IsValidDirectory("/abc/def"));
    EXPECT_FALSE(fpl.IsValidDirectory("/abc/DIR1"));
    EXPECT_TRUE(fpl.IsExistingFile("/abc/def/file0.txt"));
    EXPECT_FALSE(fpl.IsExistingFile("/abc/def/blah.txt"));

    fpl.AddPath(FilePath("/ghi/jkl/file2.txt"));
    EXPECT_EQ("/ghi/jkl/file2.txt", fpl.GetCurrent().ToString());
    EXPECT_FALSE(fpl.IsValidDirectory(fpl.GetCurrent()));
    EXPECT_TRUE(fpl.IsExistingFile(fpl.GetCurrent()));
}

TEST_F(FilePathListTest, Directions) {
    FilePathList fpl;
    fpl.Init(FilePath("/abc/def"));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kUp);
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.AddPath(FilePath("/abc/def/file0.txt"));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kBack);
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kForward);
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kHome));

    fpl.GoInDirection(FilePathList::Direction::kHome);
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kUp));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kForward));
    EXPECT_TRUE(fpl.CanGoInDirection(FilePathList::Direction::kBack));
    EXPECT_FALSE(fpl.CanGoInDirection(FilePathList::Direction::kHome));
}

TEST_F(FilePathListTest, GetContents) {
    FilePathList fpl;

    StrVec subdirs, files;

    // Start with a directory.
    fpl.Init(FilePath("/abc/def"));
    fpl.GetContents(subdirs, files, "", false);
    EXPECT_EQ(1U,          subdirs.size());
    EXPECT_EQ("subdir0",   subdirs[0]);
    EXPECT_EQ(2U,          files.size());
    EXPECT_EQ("file0.txt", files[0]);
    EXPECT_EQ("file1.txt", files[1]);

    // Start with a file - it should use the same directory as above.
    fpl.AddPath(FilePath("/abc/def/file0.txt"));
    fpl.GetContents(subdirs, files, "", false);
    EXPECT_EQ(1U,          subdirs.size());
    EXPECT_EQ("subdir0",   subdirs[0]);
    EXPECT_EQ(2U,          files.size());
    EXPECT_EQ("file0.txt", files[0]);
    EXPECT_EQ("file1.txt", files[1]);
}

TEST_F(FilePathListTest, Relative) {
    FilePathList fpl;
    fpl.Init(FilePath("/abc"));
    fpl.AddPath(FilePath("def"));
    EXPECT_EQ("/abc/def", fpl.GetCurrent().ToString());

    // Using a file should be relative to its parent directory.
    fpl.AddPath(FilePath("/abc/def/file0.txt"));
    fpl.AddPath(FilePath("subdir0"));
    EXPECT_EQ("/abc/def/subdir0", fpl.GetCurrent().ToString());
}
