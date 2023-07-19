#include <source_location>

#include "Tests/Testing.h"
#include "Util/FilePath.h"
#include "Util/UTime.h"

// Helper function that returns this source file as a string.
static std::string GetSourceFile_() {
    return std::source_location::current().file_name();
}

// Helper function that returns this source file as a FilePath.
static FilePath GetSourceFilePath_() {
    return FilePath(GetSourceFile_());
}

TEST(FilePathTest, Assign) {
    // Assign from const char *.
    FilePath p1 = "/abc/def";
    EXPECT_EQ("/abc/def", p1.ToString());
    // Assign from another FilePath.
    FilePath p2;
    p2 = p1;
    EXPECT_EQ(p1, p2);
    // Assign from string.
    FilePath p3;
    p3 = std::string("/abc/def");
    EXPECT_EQ(p1, p3);
}

TEST(FilePathTest, Clear) {
    FilePath p = "/abc/def";
    EXPECT_TRUE(p);
    p.Clear();
    EXPECT_FALSE(p);
}

TEST(FilePathTest, Exists) {
    EXPECT_FALSE(FilePath("/abc/def").Exists());
    EXPECT_TRUE(GetSourceFilePath_().Exists());
}

TEST(FilePathTest, IsDirectory) {
    EXPECT_FALSE(FilePath("/abc/def").IsDirectory());
    EXPECT_FALSE(GetSourceFilePath_().IsDirectory());
    FilePath parent =
        FilePath(std::filesystem::path(GetSourceFile_()).parent_path());
    EXPECT_TRUE(parent.IsDirectory());
}

TEST(FilePathTest, IsAbsolute) {
    EXPECT_TRUE(FilePath("/").IsAbsolute());
    EXPECT_TRUE(FilePath("/abc/def").IsAbsolute());
    EXPECT_FALSE(FilePath("abc").IsAbsolute());
    EXPECT_FALSE(FilePath("abc/def").IsAbsolute());
}

TEST(FilePathTest, GetParentDirectory) {
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

TEST(FilePathTest, GetFileName) {
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


TEST(FilePathTest, GetExtension) {
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

TEST(FilePathTest, AddExtension) {
    FilePath p = "/abc/def/ghi";
    p.AddExtension(".qrs");
    EXPECT_EQ(".qrs", p.GetExtension());
    p.AddExtension("tuv");
    EXPECT_EQ(".tuv", p.GetExtension());
}

TEST(FilePathTest, AppendRelative) {
    FilePath p1 = "abc/def";
    FilePath p2 = "qrs/tuv";
    FilePath p3 = "/abs/solute";
    EXPECT_EQ("abc/def/qrs/tuv", p2.AppendRelative(p1).ToString());
    EXPECT_EQ("/abs/solute",     p3.AppendRelative(p1).ToString());

    // Test case where the base path is a non-directory.
    const auto src_path = GetSourceFilePath_();
    EXPECT_EQ(src_path.GetParentDirectory().ToString() + "/abc/def",
              p1.AppendRelative(src_path).ToString());
}

TEST(FilePathTest, MakeRelativeTo) {
    // Absolute path relative to another absolute path.
    FilePath p1 = "/abc/def/ghijkl";
    FilePath p2 = "/abc/def/ghijkl/foo/bar.txt";
    FilePath p3 = "/abc/def/ghijkQ/foo/bar.txt";
    EXPECT_EQ("foo/bar.txt", p2.MakeRelativeTo(p1).ToString());

    // Partial sub-path match:
    EXPECT_EQ("../ghijkQ/foo/bar.txt", p3.MakeRelativeTo(p1).ToString());
}

TEST(FilePathTest, GetAbsolute) {
    const auto curdir = FilePath(std::filesystem::current_path()).ToString();

    EXPECT_EQ("/",               FilePath("/").GetAbsolute().ToString());
    EXPECT_EQ("/abc/def",        FilePath("/abc/def").GetAbsolute().ToString());
    EXPECT_EQ(curdir + "/ab",    FilePath("ab").GetAbsolute().ToString());
    EXPECT_EQ(curdir + "/ab/cd", FilePath("ab/cd").GetAbsolute().ToString());
}

TEST(FilePathTest, GetModTime) {
    const auto p = std::filesystem::path(GetSourceFile_());
    EXPECT_EQ(UTime(std::filesystem::last_write_time(p)),
              GetSourceFilePath_().GetModTime());
}

TEST(FilePathTest, GetCurrent) {
    EXPECT_EQ(FilePath(std::filesystem::current_path()),
              FilePath::GetCurrent());
}

TEST(FilePathTest, Join) {
    FilePath p1 = "/abc/def";
    FilePath p2 = "ghi/jkl";
    EXPECT_EQ("/abc/def/ghi/jkl", FilePath::Join(p1, p2).ToString());
    EXPECT_EQ("ghi/jkl/ghi/jkl",  FilePath::Join(p2, p2).ToString());
    // Originals should remain the same.
    EXPECT_EQ("/abc/def", p1.ToString());
    EXPECT_EQ("ghi/jkl",  p2.ToString());
}

TEST(FilePathTest, GetHashValue) {
    const std::string ps = "/abc/def";
    const FilePath p(ps);
    const auto hashval = std::filesystem::hash_value(std::filesystem::path(ps));
    EXPECT_EQ(hashval, p.GetHashValue());
    EXPECT_EQ(hashval, std::hash<FilePath>{}(p));
}

TEST(FilePathTest, Equality) {
    FilePath p1 = "/abc/def";
    FilePath p2 = "/abc/def";
    EXPECT_EQ(p1, p2);
    p2 = "abc/def";
    EXPECT_NE(p1, p2);
}
