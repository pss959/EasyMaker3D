#include "Testing.h"
#include "Util/Enum.h"

// Enum used for testing.
enum class TestEnum : uint32_t {
    kF1 = (1 << 0),
    kF2 = (1 << 1),
    kF3 = (1 << 2),
};

TEST(Enum, FlagsSetAndHas) {
    Util::Flags<TestEnum> flags;
    EXPECT_FALSE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestEnum::kF1));
    EXPECT_FALSE(flags.Has(TestEnum::kF2));
    EXPECT_FALSE(flags.Has(TestEnum::kF3));

    flags.Set(TestEnum::kF2);
    EXPECT_TRUE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestEnum::kF1));
    EXPECT_TRUE(flags.Has(TestEnum::kF2));
    EXPECT_FALSE(flags.Has(TestEnum::kF3));

    flags.Set(TestEnum::kF3);
    EXPECT_TRUE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestEnum::kF1));
    EXPECT_TRUE(flags.Has(TestEnum::kF2));
    EXPECT_TRUE(flags.Has(TestEnum::kF3));
}

TEST(Enum, SetAll) {
    Util::Flags<TestEnum> flags;
    flags.SetAll(true);
    EXPECT_TRUE(flags.Has(TestEnum::kF1));
    EXPECT_TRUE(flags.Has(TestEnum::kF2));
    EXPECT_TRUE(flags.Has(TestEnum::kF3));
    flags.SetAll(false);
    EXPECT_FALSE(flags.Has(TestEnum::kF1));
    EXPECT_FALSE(flags.Has(TestEnum::kF2));
    EXPECT_FALSE(flags.Has(TestEnum::kF3));
}

TEST(Enum, FlagsReset) {
    Util::Flags<TestEnum> flags;
    EXPECT_FALSE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestEnum::kF1));
    flags.Set(TestEnum::kF1);
    flags.Set(TestEnum::kF2);
    flags.Set(TestEnum::kF3);
    EXPECT_TRUE(flags.Has(TestEnum::kF1));
    EXPECT_TRUE(flags.Has(TestEnum::kF2));
    EXPECT_TRUE(flags.Has(TestEnum::kF3));
    flags.Reset(TestEnum::kF2);
    EXPECT_TRUE(flags.Has(TestEnum::kF1));
    EXPECT_FALSE(flags.Has(TestEnum::kF2));
    EXPECT_TRUE(flags.Has(TestEnum::kF3));
}

TEST(Enum, FlagsHasAnyFrom) {
    Util::Flags<TestEnum> flags1;
    Util::Flags<TestEnum> flags2;
    EXPECT_FALSE(flags1.HasAnyFrom(flags2));

    flags1.Set(TestEnum::kF2);
    flags1.Set(TestEnum::kF3);
    EXPECT_FALSE(flags1.HasAnyFrom(flags2));

    flags2.Set(TestEnum::kF1);
    EXPECT_FALSE(flags1.HasAnyFrom(flags2));

    flags2.Set(TestEnum::kF2);
    EXPECT_TRUE(flags1.HasAnyFrom(flags2));

    flags2.Set(TestEnum::kF3);
    EXPECT_TRUE(flags1.HasAnyFrom(flags2));
}

TEST(Enum, EnumName) {
    EXPECT_EQ("kF1", Util::EnumName(TestEnum::kF1));
    EXPECT_EQ("kF3", Util::EnumName(TestEnum::kF3));
}

TEST(Enum, EnumInt) {
    EXPECT_EQ(1, Util::EnumInt(TestEnum::kF1));
    EXPECT_EQ(4, Util::EnumInt(TestEnum::kF3));
}

TEST(Enum, EnumFromString) {
    TestEnum e;
    EXPECT_TRUE(Util::EnumFromString<TestEnum>("kF1", e));
    EXPECT_EQ(TestEnum::kF1, e);
    EXPECT_TRUE(Util::EnumFromString<TestEnum>("kF3", e));
    EXPECT_EQ(TestEnum::kF3, e);
    EXPECT_FALSE(Util::EnumFromString<TestEnum>("kF4", e));
}

TEST(Enum, EnumFlagNames) {
    Util::Flags<TestEnum> flags;
    flags.Set(TestEnum::kF1);
    EXPECT_EQ("kF1",     Util::EnumFlagNames(flags));
    flags.Set(TestEnum::kF3);
    EXPECT_EQ("kF1|kF3", Util::EnumFlagNames(flags));
}
