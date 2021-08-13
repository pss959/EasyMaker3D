#include "Testing.h"
#include "Util/FilePath.h"

TEST(FilePath, Concatenate) {
    Util::FilePath p1 = "abc/def";
    EXPECT_EQ("abc/def/ghi", p1 / "ghi");
    p1 /= "ghi";
    EXPECT_EQ("abc/def/ghi", p1);
}
