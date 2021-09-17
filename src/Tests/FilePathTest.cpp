#include "Testing.h"
#include "Util/FilePath.h"

TEST(FilePath, Concatenate) {
    Util::FilePath p1 = "abc/def";
    EXPECT_EQ("abc/def/ghi", p1 / "ghi");
    p1 /= "ghi";
    EXPECT_EQ("abc/def/ghi", p1);
}

TEST(FilePath, MakeRelativeTo) {
    Util::FilePath p1 = "abc/def";
    Util::FilePath p2 = "qrs/tuv";
    Util::FilePath p3 = "/abs/solute";
    EXPECT_EQ("abc/def/qrs/tuv", p2.MakeRelativeTo(p1));
    EXPECT_EQ("/abs/solute",     p3.MakeRelativeTo(p1));
}
