#include "Testing.h"
#include "Util/Enum.h"
#include "Util/Flags.h"

namespace {

// Enum used for testing.
enum class TestEnum { kSomething, kAnother, kAlso = 7 };

TEST(Enum, EnumName) {
    EXPECT_EQ("kSomething", Util::EnumName(TestEnum::kSomething));
    EXPECT_EQ("kAlso",      Util::EnumName(TestEnum::kAlso));
}

TEST(Enum, EnumInt) {
    EXPECT_EQ(0, Util::EnumInt(TestEnum::kSomething));
    EXPECT_EQ(1, Util::EnumInt(TestEnum::kAnother));
    EXPECT_EQ(7, Util::EnumInt(TestEnum::kAlso));
}

TEST(Enum, EnumValues) {
    auto values = Util::EnumValues<TestEnum>();
    EXPECT_EQ(3U, values.size());
    EXPECT_EQ(TestEnum::kSomething, values[0]);
    EXPECT_EQ(TestEnum::kAnother,   values[1]);
    EXPECT_EQ(TestEnum::kAlso,      values[2]);
}

TEST(Enum, EnumFromString) {
    TestEnum e;
    EXPECT_TRUE(Util::EnumFromString<TestEnum>("kSomething", e));
    EXPECT_EQ(TestEnum::kSomething, e);
    EXPECT_TRUE(Util::EnumFromString<TestEnum>("kAlso", e));
    EXPECT_EQ(TestEnum::kAlso, e);
    EXPECT_FALSE(Util::EnumFromString<TestEnum>("kAlsop", e));
    EXPECT_FALSE(Util::EnumFromString<TestEnum>("xkAlso", e));
}

}  // anonymous namespace
