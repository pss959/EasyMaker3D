#include "Testing.h"
#include "Util/String.h"

TEST(String, ToString) {
    EXPECT_EQ("1234", Util::ToString(1234));
    EXPECT_EQ("5",    Util::ToString(5.f));
}

TEST(String, StringsEqualNoCase) {
    EXPECT_TRUE(Util::StringsEqualNoCase(" ab c", " ab c"));
    EXPECT_TRUE(Util::StringsEqualNoCase(" ab c", " AB C"));
    EXPECT_TRUE(Util::StringsEqualNoCase(" ab c", " aB C"));
    EXPECT_FALSE(Util::StringsEqualNoCase(" ab c", " ab d"));
}
