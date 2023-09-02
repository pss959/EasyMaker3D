#include <source_location>

#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/FileSystem.h"
#include "Util/General.h"

/// \ingroup Tests
class FileSystemTest : public TestBase {
  protected:
    // Shorthand.
    using FSPath_ = FileSystem::Path;

    /// Use a real FileSystem for these tests.
    FileSystemPtr fs;

    FileSystemTest() {
        /// Use a real FileSystem and make it easily accessible in tests.
        UseRealFileSystem(true);
        fs = FileSystem::GetInstalled();
        EXPECT_FALSE(fs->IsFake());
    }

    /// Returns this source file as a string.
    static Str GetSourceFile_() {
        return std::source_location::current().file_name();
    }

    /// Returns this source file as a FileSystem::Path; this is known to be an
    /// existing file.
    static FSPath_ GetSourceFilePath_() {
        return FSPath_(GetSourceFile_());
    }
};

TEST_F(FileSystemTest, ToNativeString) {
    // From the make_preferred() documentation:
    //   On Windows:
    //     "a\\b\\c" -> "a\\b\\c"
    //     "a/b/c"   -> "a\\b\\c"
    // on POSIX
    //     "a\\b\\c" -> "a\\b\\c"
    //     "a/b/c"   -> "a/b/c"

#ifdef ION_PLATFORM_WINDOWS
    EXPECT_EQ("a\\b\\c", fs->ToNativeString("a\\b\\c"));
    EXPECT_EQ("a\\b\\c", fs->ToNativeString("a/b/c"));
#else
    EXPECT_EQ("a\\b\\c", fs->ToNativeString("a\\b\\c"));
    EXPECT_EQ("a/b/c",   fs->ToNativeString("a/b/c"));
#endif
}

TEST_F(FileSystemTest, Exists) {
    EXPECT_FALSE(fs->Exists(FSPath_("/abc/def")));
    EXPECT_TRUE(fs->Exists(GetSourceFilePath_()));
}

TEST_F(FileSystemTest, IsDirectory) {
    EXPECT_FALSE(fs->IsDirectory(FSPath_("/abc/def")));
    EXPECT_FALSE(fs->IsDirectory(GetSourceFilePath_()));
    EXPECT_TRUE(fs->IsDirectory(GetSourceFilePath_().parent_path()));
}

TEST_F(FileSystemTest, IsAbsolute) {
    EXPECT_TRUE(fs->IsAbsolute(FSPath_("/")));
    EXPECT_TRUE(fs->IsAbsolute(FSPath_("/abc.def")));
    EXPECT_FALSE(fs->IsAbsolute(FSPath_("abc")));
    EXPECT_FALSE(fs->IsAbsolute(FSPath_("abc/def")));
}

TEST_F(FileSystemTest, IsHidden) {
    EXPECT_FALSE(fs->IsHidden(GetSourceFilePath_()));
}

TEST_F(FileSystemTest, GetModTime) {
    const auto p = GetSourceFilePath_();
    EXPECT_EQ(UTime(std::filesystem::last_write_time(p)), fs->GetModTime(p));
}

TEST_F(FileSystemTest, GetDirectoryContents) {
    // The Tests/Data/Files directory has the following contents.
    //   Tests/Data/Files/
    //       subdir0/
    //           .hidden0.txt
    //           a.txt
    //           b.txt
    //       subdir1/
    //           c.txt
    //       d.txt

    Str dir = GetDataPath("Files").ToString();
    EXPECT_TRUE(fs->IsDirectory(dir));
    StrVec files, subdirs;
    fs->GetDirectoryContents(dir, subdirs, files, false);  // Skip hidden.
    EXPECT_EQ(2U,        subdirs.size());
    EXPECT_EQ("subdir0", subdirs[0]);
    EXPECT_EQ("subdir1", subdirs[1]);
    EXPECT_EQ(1U,        files.size());
    EXPECT_EQ("d.txt",   files[0]);

    dir += "/subdir0";
    fs->GetDirectoryContents(dir, subdirs, files, false);  // Skip hidden
    EXPECT_EQ(0U,      subdirs.size());
    EXPECT_EQ(2U,      files.size());
    EXPECT_EQ("a.txt", files[0]);
    EXPECT_EQ("b.txt", files[1]);

    fs->GetDirectoryContents(dir, subdirs, files, true);   // Include hidden
    EXPECT_EQ(0U,             subdirs.size());
    EXPECT_EQ(3U,             files.size());
    EXPECT_EQ(".hidden0.txt", files[0]);
    EXPECT_EQ("a.txt",        files[1]);
    EXPECT_EQ("b.txt",        files[2]);
}
