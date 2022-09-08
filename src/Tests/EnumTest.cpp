#include "Testing.h"
#include "Util/Enum.h"
#include "Util/Flags.h"

namespace {

// Enum used for testing.
enum class TestEnum { kSomething, kAnOther, kABCMore, kAlso = 7 };

TEST(Enum, EnumName) {
    EXPECT_EQ("kSomething", Util::EnumName(TestEnum::kSomething));
    EXPECT_EQ("kAnOther",   Util::EnumName(TestEnum::kAnOther));
    EXPECT_EQ("kABCMore",   Util::EnumName(TestEnum::kABCMore));
    EXPECT_EQ("kAlso",      Util::EnumName(TestEnum::kAlso));
}

TEST(Enum, EnumInt) {
    EXPECT_EQ(0, Util::EnumInt(TestEnum::kSomething));
    EXPECT_EQ(1, Util::EnumInt(TestEnum::kAnOther));
    EXPECT_EQ(2, Util::EnumInt(TestEnum::kABCMore));
    EXPECT_EQ(7, Util::EnumInt(TestEnum::kAlso));
}

TEST(Enum, EnumFromIndex) {
    EXPECT_EQ(TestEnum::kSomething, Util::EnumFromIndex<TestEnum>(0));
    EXPECT_EQ(TestEnum::kAnOther,   Util::EnumFromIndex<TestEnum>(1));
    EXPECT_EQ(TestEnum::kABCMore,   Util::EnumFromIndex<TestEnum>(2));
    EXPECT_EQ(TestEnum::kAlso,      Util::EnumFromIndex<TestEnum>(3));
}

TEST(Enum, EnumValues) {
    auto values = Util::EnumValues<TestEnum>();
    EXPECT_EQ(4U, values.size());
    EXPECT_EQ(TestEnum::kSomething, values[0]);
    EXPECT_EQ(TestEnum::kAnOther,   values[1]);
    EXPECT_EQ(TestEnum::kABCMore,   values[2]);
    EXPECT_EQ(TestEnum::kAlso,      values[3]);
}

TEST(Enum, EnumFromString) {
    TestEnum e;
    EXPECT_TRUE(Util::EnumFromString<TestEnum>("kSomething", e));
    EXPECT_EQ(TestEnum::kSomething, e);
    EXPECT_TRUE(Util::EnumFromString<TestEnum>("kABCMore", e));
    EXPECT_EQ(TestEnum::kABCMore, e);
    EXPECT_TRUE(Util::EnumFromString<TestEnum>("kAlso", e));
    EXPECT_EQ(TestEnum::kAlso, e);
    EXPECT_FALSE(Util::EnumFromString<TestEnum>("kAlsop", e));
    EXPECT_FALSE(Util::EnumFromString<TestEnum>("xkAlso", e));
}

TEST(Enum, EnumToWord) {
    EXPECT_EQ("Something", Util::EnumToWord(TestEnum::kSomething));
    EXPECT_EQ("AnOther",   Util::EnumToWord(TestEnum::kAnOther));
    EXPECT_EQ("ABCMore",   Util::EnumToWord(TestEnum::kABCMore));
    EXPECT_EQ("Also",      Util::EnumToWord(TestEnum::kAlso));
}

TEST(Enum, EnumToWords) {
    EXPECT_EQ("Something", Util::EnumToWords(TestEnum::kSomething));
    EXPECT_EQ("An Other",  Util::EnumToWords(TestEnum::kAnOther));
    EXPECT_EQ("ABC More",  Util::EnumToWords(TestEnum::kABCMore));
    EXPECT_EQ("Also",      Util::EnumToWords(TestEnum::kAlso));
}

}  // anonymous namespace
