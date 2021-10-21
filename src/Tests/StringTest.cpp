#include "Testing.h"
#include "Util/String.h"

TEST(String, ToString) {
    EXPECT_EQ("1234", Util::ToString(1234));
    EXPECT_EQ("5",    Util::ToString(5.f));
}

TEST(String, StringContains) {
    EXPECT_TRUE(Util::StringContains("ab c", "ab"));
    EXPECT_FALSE(Util::StringContains("ab c", "Ab"));
    EXPECT_TRUE(Util::StringContains("ab c", " c"));
    EXPECT_TRUE(Util::StringContains("ab c", "c"));
    EXPECT_FALSE(Util::StringContains("ab c", "abc"));
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

TEST(String, ReplaceString) {
    EXPECT_EQ("HeXXo WorXd", Util::ReplaceString("Hello World", "l", "X"));
    EXPECT_EQ("Hello World", Util::ReplaceString("Hello World", "v", "X"));
    EXPECT_EQ("Nothing",     Util::ReplaceString("Hello World", "Hello World",
                                                 "Nothing"));
}

TEST(String, JoinStrings) {
    std::vector<std::string> v{ "ABC", "DEF", "GHI" };
    EXPECT_EQ("ABC DEF GHI",   Util::JoinStrings(v));
    EXPECT_EQ("ABCDEFGHI",     Util::JoinStrings(v, ""));
    EXPECT_EQ("ABC, DEF, GHI", Util::JoinStrings(v, ", "));
}

TEST(String, JoinItems) {
    std::vector<int> v{ 12, 13, 14 };
    EXPECT_EQ("12 13 14",   Util::JoinItems(v));
    EXPECT_EQ("121314",     Util::JoinItems(v, ""));
    EXPECT_EQ("12, 13, 14", Util::JoinItems(v, ", "));
}
