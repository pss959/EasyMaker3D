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

TEST(String, CompareStrings) {
    size_t index;
    EXPECT_TRUE(Util::CompareStrings("abcd", "abcd", index));
    EXPECT_FALSE(Util::CompareStrings("abcd", "abce", index));
    EXPECT_EQ(3U, index);
    EXPECT_FALSE(Util::CompareStrings("abcd", "Xbcd", index));
    EXPECT_EQ(0U, index);
    EXPECT_FALSE(Util::CompareStrings("abc\ndef", "abc def", index));
    EXPECT_EQ(3U, index);
    EXPECT_FALSE(Util::CompareStrings("abcde", "ab", index));
    EXPECT_EQ(2U, index);
    EXPECT_FALSE(Util::CompareStrings("abcd", "abcde", index));
    EXPECT_EQ(4U, index);
}
