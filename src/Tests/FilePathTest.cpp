#include "Tests/Testing.h"
#include "Util/FilePath.h"

TEST(FilePath, Join) {
    FilePath p1 = "/abc/def";
    FilePath p2 = "ghi/jkl";
    EXPECT_EQ("/abc/def/ghi/jkl", FilePath::Join(p1, p2).ToString());
    EXPECT_EQ("ghi/jkl/ghi/jkl",  FilePath::Join(p2, p2).ToString());
    // Originals should remain the same.
    EXPECT_EQ("/abc/def", p1.ToString());
    EXPECT_EQ("ghi/jkl",  p2.ToString());
}

TEST(FilePath, AppendRelative) {
    FilePath p1 = "abc/def";
    FilePath p2 = "qrs/tuv";
    FilePath p3 = "/abs/solute";
    EXPECT_EQ("abc/def/qrs/tuv", p2.AppendRelative(p1).ToString());
    EXPECT_EQ("/abs/solute",     p3.AppendRelative(p1).ToString());
}

TEST(FilePath, MakeRelativeTo) {
    // Absolute path relative to another absolute path.
    FilePath p1 = "/abc/def/ghijkl";
    FilePath p2 = "/abc/def/ghijkl/foo/bar.txt";
    FilePath p3 = "/abc/def/ghijkQ/foo/bar.txt";
    EXPECT_EQ("foo/bar.txt", p2.MakeRelativeTo(p1).ToString());

    // Partial sub-path match:
    EXPECT_EQ("../ghijkQ/foo/bar.txt", p3.MakeRelativeTo(p1).ToString());
}

TEST(FilePath, Equality) {
    FilePath p1 = "/abc/def";
    FilePath p2 = "/abc/def";
    EXPECT_EQ(p1, p2);
    p2 = "abc/def";
    EXPECT_NE(p1, p2);
}

TEST(FilePath, GetFileName) {
    FilePath p = "/abc/def/ghi.jkl";
    EXPECT_EQ("ghi.jkl", p.GetFileName());
    EXPECT_EQ("ghi",     p.GetFileName(true));
}

TEST(FilePath, Extension) {
    FilePath p = "/abc/def/ghi.jkl";
    EXPECT_EQ(".jkl", p.GetExtension());
    p = "/abc/def/ghi";
    EXPECT_EQ("", p.GetExtension());
    p.AddExtension(".qrs");
    EXPECT_EQ(".qrs", p.GetExtension());
    p.AddExtension("tuv");
    EXPECT_EQ(".tuv", p.GetExtension());
}
