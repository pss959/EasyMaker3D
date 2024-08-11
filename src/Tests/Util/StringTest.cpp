//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Testing.h"
#include "Util/String.h"

TEST(StringTest, ToString) {
    EXPECT_EQ("1234", Util::ToString(1234));
    EXPECT_EQ("5",    Util::ToString(5.f));

    EXPECT_EQ("1234",   Util::ToString(1234, 0));
    EXPECT_EQ("1234",   Util::ToString(1234, 2));
    EXPECT_EQ("1234",   Util::ToString(1234, 4));
    EXPECT_EQ("1234 ",  Util::ToString(1234, 5));
    EXPECT_EQ("1234  ", Util::ToString(1234, 6));

    // Sepcial case for bool.
    EXPECT_EQ("True",  Util::ToString(true));
    EXPECT_EQ("False", Util::ToString(false));
    EXPECT_EQ("T",     Util::ToString(true,  true));  // Shorten.
    EXPECT_EQ("F",     Util::ToString(false, true));  // Shorten.
}

TEST(StringTest, ToUpperCase) {
    EXPECT_EQ("ABCD", Util::ToUpperCase("ABCD"));
    EXPECT_EQ("ABCD", Util::ToUpperCase("abcd"));
    EXPECT_EQ("ABCD", Util::ToUpperCase("aBcD"));
    EXPECT_EQ("R2D2", Util::ToUpperCase("r2d2"));
    EXPECT_EQ("A  B", Util::ToUpperCase("a  b"));
}

TEST(StringTest, ToLowerCase) {
    EXPECT_EQ("abcd", Util::ToLowerCase("ABCD"));
    EXPECT_EQ("abcd", Util::ToLowerCase("abcd"));
    EXPECT_EQ("abcd", Util::ToLowerCase("aBcD"));
    EXPECT_EQ("r2d2", Util::ToLowerCase("R2D2"));
    EXPECT_EQ("a  b", Util::ToLowerCase("A  B"));
}

TEST(StringTest, StringsEqualNoCase) {
    EXPECT_TRUE(Util::StringsEqualNoCase(" ab c", " ab c"));
    EXPECT_TRUE(Util::StringsEqualNoCase(" ab c", " AB C"));
    EXPECT_TRUE(Util::StringsEqualNoCase(" ab c", " aB C"));
    EXPECT_FALSE(Util::StringsEqualNoCase(" ab c", " ab d"));
}

TEST(StringTest, CompareStrings) {
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

TEST(StringTest, ReplaceString) {
    EXPECT_EQ("HeXXo WorXd", Util::ReplaceString("Hello World", "l", "X"));
    EXPECT_EQ("Hello World", Util::ReplaceString("Hello World", "v", "X"));
    EXPECT_EQ("Nothing",     Util::ReplaceString("Hello World", "Hello World",
                                                 "Nothing"));
}

TEST(StringTest, JoinStrings) {
    StrVec v{ "ABC", "DEF", "GHI" };
    EXPECT_EQ("ABC DEF GHI",   Util::JoinStrings(v));
    EXPECT_EQ("ABCDEFGHI",     Util::JoinStrings(v, ""));
    EXPECT_EQ("ABC, DEF, GHI", Util::JoinStrings(v, ", "));
}

TEST(StringTest, JoinItems) {
    std::vector<int> v{ 12, 13, 14 };
    EXPECT_EQ("12 13 14",   Util::JoinItems(v));
    EXPECT_EQ("121314",     Util::JoinItems(v, ""));
    EXPECT_EQ("12, 13, 14", Util::JoinItems(v, ", "));
}

TEST(StringTest, RemoveFirstN) {
    const Str s = "abcdefghi";
    EXPECT_EQ("abcdefghi", Util::RemoveFirstN(s, 0));
    EXPECT_EQ("bcdefghi",  Util::RemoveFirstN(s, 1));
    EXPECT_EQ("cdefghi",   Util::RemoveFirstN(s, 2));
    EXPECT_EQ("i",         Util::RemoveFirstN(s, 8));
    EXPECT_EQ("",          Util::RemoveFirstN(s, 9));
    EXPECT_EQ("",          Util::RemoveFirstN(s, 10));
}

TEST(StringTest, SplitCamelCase) {
    EXPECT_EQ("Hello There",   Util::SplitCamelCase("HelloThere",  false));
    EXPECT_EQ("hello There",   Util::SplitCamelCase("helloThere",  false));
    EXPECT_EQ("Abc Def GH Ij", Util::SplitCamelCase("AbcDefGHIj",  false));
    EXPECT_EQ("Hello There",   Util::SplitCamelCase("kHelloThere", true));
}

TEST(StringTest, StringToInteger) {
    int n;
    EXPECT_TRUE(Util::StringToInteger("0", n));
    EXPECT_EQ(0, n);
    EXPECT_TRUE(Util::StringToInteger("1", n));
    EXPECT_EQ(1, n);
    EXPECT_TRUE(Util::StringToInteger("12345", n));
    EXPECT_EQ(12345, n);
    EXPECT_TRUE(Util::StringToInteger("+0", n));
    EXPECT_EQ(0, n);
    EXPECT_TRUE(Util::StringToInteger("+1", n));
    EXPECT_EQ(1, n);
    EXPECT_TRUE(Util::StringToInteger("+12345", n));
    EXPECT_EQ(12345, n);
    EXPECT_TRUE(Util::StringToInteger("-0", n));
    EXPECT_EQ(0, n);
    EXPECT_TRUE(Util::StringToInteger("-1", n));
    EXPECT_EQ(-1, n);
    EXPECT_TRUE(Util::StringToInteger("-12345", n));
    EXPECT_EQ(-12345, n);

    // Error cases.
    EXPECT_FALSE(Util::StringToInteger(" 0",  n));
    EXPECT_FALSE(Util::StringToInteger("0 ",  n));
    EXPECT_FALSE(Util::StringToInteger("0.",  n));
    EXPECT_FALSE(Util::StringToInteger("a",   n));
    EXPECT_FALSE(Util::StringToInteger("0a",  n));
    EXPECT_FALSE(Util::StringToInteger("a0",  n));
    EXPECT_FALSE(Util::StringToInteger("1z2", n));
}
