#include "Testing.h"
#include "Util/FilePath.h"

TEST(FilePath, Concatenate) {
    Util::FilePath p1 = "abc/def";
    EXPECT_EQ("abc/def/ghi", p1 / "ghi");
    p1 /= "ghi";
    EXPECT_EQ("abc/def/ghi", p1);
}

TEST(FilePath, JoinPaths) {
    Util::FilePath p1 = "/abc/def";
    Util::FilePath p2 = "ghi/jkl";
    EXPECT_EQ("/abc/def/ghi/jkl", Util::FilePath::JoinPaths(p1, p2));
    EXPECT_EQ("ghi/jkl/ghi/jkl",  Util::FilePath::JoinPaths(p2, p2));
    // Originals should remain the same.
    EXPECT_EQ("/abc/def", p1);
    EXPECT_EQ("ghi/jkl",  p2);
}

TEST(FilePath, MakeRelativeTo) {
    Util::FilePath p1 = "abc/def";
    Util::FilePath p2 = "qrs/tuv";
    Util::FilePath p3 = "/abs/solute";
    EXPECT_EQ("abc/def/qrs/tuv", p2.MakeRelativeTo(p1));
    EXPECT_EQ("/abs/solute",     p3.MakeRelativeTo(p1));
}

TEST(FilePath, Equality) {
    Util::FilePath p1 = "/abc/def";
    Util::FilePath p2 = "/abc/def";
    EXPECT_EQ(p1, p2);
    p2 = "abc/def";
    EXPECT_NE(p1, p2);
}
