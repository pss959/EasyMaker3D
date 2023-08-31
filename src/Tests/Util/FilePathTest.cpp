#include <source_location>

#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Tests/Util/FakeFileSystem.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/UTime.h"

/// \ingroup Tests
class FilePathTest : public TestBase {
  protected:
    /// Make the FakeFileSystem easily accessible to tests.
    std::shared_ptr<FakeFileSystem> fs;

    FilePathTest() { fs = GetFakeFileSystem(); }
};

TEST_F(FilePathTest, Assign) {
    // Assign from const char *.
    FilePath p1 = "/abc/def";
    EXPECT_EQ("/abc/def", p1.ToString());
    // Assign from another FilePath.
    FilePath p2;
    p2 = p1;
    EXPECT_EQ(p1, p2);
    // Assign from string.
    FilePath p3;
    p3 = Str("/abc/def");
    EXPECT_EQ(p1, p3);
}

TEST_F(FilePathTest, Clear) {
    FilePath p = "/abc/def";
    EXPECT_TRUE(p);
    p.Clear();
    EXPECT_FALSE(p);
}

TEST_F(FilePathTest, Exists) {
    fs->AddFile("/abc/def");
    EXPECT_TRUE(FilePath("/abc/def").Exists());
    EXPECT_FALSE(FilePath("/abc/defg").Exists());
}

TEST_F(FilePathTest, IsDirectory) {
    fs->AddFile("/abc/def");
    fs->AddDir("/abc/ghi");
    EXPECT_FALSE(FilePath("/abc/def").IsDirectory());
    EXPECT_TRUE(FilePath("/abc/ghi").IsDirectory());
}

TEST_F(FilePathTest, IsAbsolute) {
    EXPECT_TRUE(FilePath("/").IsAbsolute());
    EXPECT_TRUE(FilePath("/abc/def").IsAbsolute());
    EXPECT_FALSE(FilePath("abc").IsAbsolute());
    EXPECT_FALSE(FilePath("abc/def").IsAbsolute());
}

TEST_F(FilePathTest, IsHidden) {
    // Files have to exist and be marked as hidden.
    fs->AddFile("abc",  false);  // Not hidden.
    fs->AddFile(".def", true);   // Hidden.
    fs->AddDir("ghi",   false);  // Not hidden.
    fs->AddDir(".jkl",  true);   // Hidden.
    EXPECT_FALSE(FilePath("notexist").IsHidden());
    EXPECT_FALSE(FilePath("abc").IsHidden());
    EXPECT_FALSE(FilePath(".abc").IsHidden());
    EXPECT_TRUE(FilePath(".def").IsHidden());
    EXPECT_FALSE(FilePath("ghi").IsHidden());
    EXPECT_TRUE(FilePath(".jkl").IsHidden());
}

TEST_F(FilePathTest, GetParentDirectory) {
    EXPECT_EQ("/", FilePath("/").GetParentDirectory().ToString());
    EXPECT_EQ("/", FilePath("/ab").GetParentDirectory().ToString());
    EXPECT_EQ("/ab", FilePath("/ab/cd").GetParentDirectory().ToString());
    EXPECT_EQ("/ab/cd", FilePath("/ab/cd/").GetParentDirectory().ToString());
    EXPECT_EQ("/ab/cd", FilePath("/ab/cd/ef").GetParentDirectory().ToString());
    EXPECT_EQ("", FilePath("ab").GetParentDirectory().ToString());
    EXPECT_EQ("ab", FilePath("ab/").GetParentDirectory().ToString());
    EXPECT_EQ("ab", FilePath("ab/cd").GetParentDirectory().ToString());
    EXPECT_EQ("ab/cd", FilePath("ab/cd/").GetParentDirectory().ToString());
    EXPECT_EQ("ab/cd", FilePath("ab/cd/ef").GetParentDirectory().ToString());
}

TEST_F(FilePathTest, GetFileName) {
    // Leave extension.
    EXPECT_EQ("",      FilePath("/").GetFileName());
    EXPECT_EQ("ab.ex", FilePath("/ab.ex").GetFileName());
    EXPECT_EQ("cd.ex", FilePath("/ab/cd.ex").GetFileName());
    EXPECT_EQ("",      FilePath("/ab/cd/").GetFileName());
    EXPECT_EQ("ef.ex", FilePath("/ab/cd/ef.ex").GetFileName());
    EXPECT_EQ("ab.ex", FilePath("ab.ex").GetFileName());
    EXPECT_EQ("",      FilePath("ab/").GetFileName());
    EXPECT_EQ("cd.ex", FilePath("ab/cd.ex").GetFileName());
    EXPECT_EQ("",      FilePath("ab/cd/").GetFileName());
    EXPECT_EQ("ef.ex", FilePath("ab/cd/ef.ex").GetFileName());

    // Remove extension.
    EXPECT_EQ("",   FilePath("/").GetFileName(true));
    EXPECT_EQ("ab", FilePath("/ab.ex").GetFileName(true));
    EXPECT_EQ("cd", FilePath("/ab/cd.ex").GetFileName(true));
    EXPECT_EQ("",   FilePath("/ab/cd/").GetFileName(true));
    EXPECT_EQ("ef", FilePath("/ab/cd/ef.ex").GetFileName(true));
    EXPECT_EQ("ab", FilePath("ab.ex").GetFileName(true));
    EXPECT_EQ("",   FilePath("ab/").GetFileName(true));
    EXPECT_EQ("cd", FilePath("ab/cd.ex").GetFileName(true));
    EXPECT_EQ("",   FilePath("ab/cd/").GetFileName(true));
    EXPECT_EQ("ef", FilePath("ab/cd/ef.ex").GetFileName(true));
}


TEST_F(FilePathTest, GetExtension) {
    EXPECT_EQ("",    FilePath("/").GetExtension());
    EXPECT_EQ(".ex", FilePath("/ab.ex").GetExtension());
    EXPECT_EQ(".ex", FilePath("/ab/cd.ex").GetExtension());
    EXPECT_EQ("",    FilePath("/ab/cd/").GetExtension());
    EXPECT_EQ(".ex", FilePath("/ab/cd/ef.ex").GetExtension());
    EXPECT_EQ(".ex", FilePath("ab.ex").GetExtension());
    EXPECT_EQ("",    FilePath("ab/").GetExtension());
    EXPECT_EQ(".ex", FilePath("ab/cd.ex").GetExtension());
    EXPECT_EQ("",    FilePath("ab/cd/").GetExtension());
    EXPECT_EQ(".ex", FilePath("ab/cd/ef.ex").GetExtension());
}

TEST_F(FilePathTest, AddExtension) {
    FilePath p = "/abc/def/ghi";
    p.AddExtension(".qrs");
    EXPECT_EQ(".qrs", p.GetExtension());
    p.AddExtension("tuv");
    EXPECT_EQ(".tuv", p.GetExtension());
}

TEST_F(FilePathTest, AppendRelative) {
    const FilePath p1 = "abc/def";
    const FilePath p2 = "qrs/tuv";
    const FilePath p3 = "/abs/solute";
    EXPECT_EQ("abc/def/qrs/tuv", p2.AppendRelative(p1).ToString());
    EXPECT_EQ("/abs/solute",     p3.AppendRelative(p1).ToString());

    // Test case where the base path is a non-directory.
    const FilePath p4 = "/mno/pqr.jpg";
    fs->AddFile(p4.ToString());
    EXPECT_EQ("/mno/abc/def", p1.AppendRelative(p4).ToString());
}

TEST_F(FilePathTest, MakeRelativeTo) {
    // Absolute path relative to another absolute path.
    FilePath p1 = "/abc/def/ghijkl";
    FilePath p2 = "/abc/def/ghijkl/foo/bar.txt";
    FilePath p3 = "/abc/def/ghijkQ/foo/bar.txt";
    EXPECT_EQ("foo/bar.txt", p2.MakeRelativeTo(p1).ToString());

    // Partial sub-path match:
    EXPECT_EQ("../ghijkQ/foo/bar.txt", p3.MakeRelativeTo(p1).ToString());
}

TEST_F(FilePathTest, GetAbsolute) {
    const Str dir = "/some/dir";
    fs->AddDir(dir);
    fs->MakeCurrent(dir);

    EXPECT_EQ("/",            FilePath("/").GetAbsolute().ToString());
    EXPECT_EQ("/abc/def",     FilePath("/abc/def").GetAbsolute().ToString());
    EXPECT_EQ(dir + "/ab",    FilePath("ab").GetAbsolute().ToString());
    EXPECT_EQ(dir + "/ab/cd", FilePath("ab/cd").GetAbsolute().ToString());
}

TEST_F(FilePathTest, GetModTime) {
    const UTime time = UTime::Now();

    const Str f = "/abc/def";
    fs->AddFile(f);
    fs->SetModTime(f, time);
    EXPECT_EQ(time, FilePath(f).GetModTime());
}

TEST_F(FilePathTest, GetContents) {
    fs->AddDir("dir0");
    fs->AddDir("dir1");
    fs->AddFile("a.txt");
    fs->AddDir("dir0/subd");
    fs->AddFile("dir0/b.txt");
    fs->AddFile("dir0/c.txt");
    fs->AddFile("dir0/d.jpg");

    StrVec subdirs, files;

    // Empty directory.
    FilePath p("dir1");
    p.GetContents(subdirs, files, "", false);
    EXPECT_EQ(0U, subdirs.size());
    EXPECT_EQ(0U, files.size());

    // Directory with contents.
    p = "dir0";
    p.GetContents(subdirs, files, "", false);
    EXPECT_EQ(1U,      subdirs.size());
    EXPECT_EQ("subd",  subdirs[0]);
    EXPECT_EQ(3U,      files.size());
    EXPECT_EQ("b.txt", files[0]);
    EXPECT_EQ("c.txt", files[1]);
    EXPECT_EQ("d.jpg", files[2]);

    // Directory with contents, filtered by extension.
    p = "dir0";
    p.GetContents(subdirs, files, ".txt", false);
    EXPECT_EQ(1U,      subdirs.size());
    EXPECT_EQ("subd",  subdirs[0]);
    EXPECT_EQ(2U,      files.size());
    EXPECT_EQ("b.txt", files[0]);
    EXPECT_EQ("c.txt", files[1]);

    // A non-directory returns nothing.
    p = "a.txt";
    p.GetContents(subdirs, files, ".txt", false);
    EXPECT_EQ(0U, subdirs.size());
    EXPECT_EQ(0U, files.size());
}

TEST_F(FilePathTest, Remove) {
    FilePath p("/some/path.txt");
    fs->AddFile(p.ToString());
    EXPECT_TRUE(p.Exists());
    p.Remove();
    EXPECT_FALSE(p.Exists());
}

TEST_F(FilePathTest, CreateDirectories) {
    EXPECT_FALSE(fs->IsDirectory("/abc"));
    EXPECT_FALSE(fs->IsDirectory("/abc/def"));
    EXPECT_FALSE(fs->IsDirectory("/abc/def/ghi"));

    FilePath p("/abc/def/ghi");
    EXPECT_TRUE(p.CreateDirectories());
    EXPECT_TRUE(fs->IsDirectory("/abc"));
    EXPECT_TRUE(fs->IsDirectory("/abc/def"));
    EXPECT_TRUE(fs->IsDirectory("/abc/def/ghi"));

    // Already exists = not an error.
    EXPECT_TRUE(p.CreateDirectories());
}

TEST_F(FilePathTest, Current) {
    FilePath dir("/some/dir");
    fs->AddDir(dir.ToString());
    dir.MakeCurrent();
    EXPECT_EQ(dir, FilePath::GetCurrent());
}

TEST_F(FilePathTest, Join) {
    FilePath p1 = "/abc/def";
    FilePath p2 = "ghi/jkl";
    EXPECT_EQ("/abc/def/ghi/jkl", FilePath::Join(p1, p2).ToString());
    EXPECT_EQ("ghi/jkl/ghi/jkl",  FilePath::Join(p2, p2).ToString());
    // Originals should remain the same.
    EXPECT_EQ("/abc/def", p1.ToString());
    EXPECT_EQ("ghi/jkl",  p2.ToString());
}

TEST_F(FilePathTest, ResourcePaths) {
    const Str rdir = RESOURCE_DIR;
    EXPECT_EQ(rdir, FilePath::GetResourceBasePath());
    EXPECT_EQ(rdir + "/foo/bar.txt",
              FilePath::GetResourcePath("foo", "bar.txt"));
    EXPECT_EQ(rdir + "/foo/bar.txt",
              FilePath::GetFullResourcePath("foo", "bar.txt"));
    EXPECT_EQ("/foo/bar.txt",
              FilePath::GetFullResourcePath("/foo", "bar.txt"));
    EXPECT_EQ("/bar.txt",
              FilePath::GetFullResourcePath("/foo", "/bar.txt"));
}

TEST_F(FilePathTest, OtherPaths) {
    EXPECT_EQ("/home/user",     FilePath::GetHomeDirPath());
    EXPECT_EQ("/settings/Blah", FilePath::GetSettingsDirPath("Blah"));
    EXPECT_EQ(TEST_DATA_DIR,    FilePath::GetTestDataPath());
}

TEST_F(FilePathTest, GetSeparator) {
    EXPECT_EQ("/", FilePath::GetSeparator());
}

TEST_F(FilePathTest, GetHashValue) {
    const Str ps = "/abc/def";
    const FilePath p(ps);
    const auto hashval = std::filesystem::hash_value(std::filesystem::path(ps));
    EXPECT_EQ(hashval, p.GetHashValue());
    EXPECT_EQ(hashval, std::hash<FilePath>{}(p));
}

TEST_F(FilePathTest, Equality) {
    FilePath p1 = "/abc/def";
    FilePath p2 = "/abc/def";
    EXPECT_EQ(p1, p2);
    p2 = "abc/def";
    EXPECT_NE(p1, p2);
}
