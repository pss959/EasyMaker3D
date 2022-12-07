#include "Tests/Testing.h"
#include "Util/Flags.h"

namespace {

// Enum used for testing.
enum class TestFlags : uint32_t {
    kF1 = (1 << 0),
    kF2 = (1 << 1),
    kF3 = (1 << 2),
};

TEST(Flags, FlagsSetAndHas) {
    Util::Flags<TestFlags> flags;
    EXPECT_FALSE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestFlags::kF1));
    EXPECT_FALSE(flags.Has(TestFlags::kF2));
    EXPECT_FALSE(flags.Has(TestFlags::kF3));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF1));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF2));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF3));

    flags.Set(TestFlags::kF2);
    EXPECT_TRUE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestFlags::kF1));
    EXPECT_TRUE(flags.Has(TestFlags::kF2));
    EXPECT_FALSE(flags.Has(TestFlags::kF3));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF1));
    EXPECT_TRUE(flags.HasOnly(TestFlags::kF2));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF3));

    flags.Set(TestFlags::kF3);
    EXPECT_TRUE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestFlags::kF1));
    EXPECT_TRUE(flags.Has(TestFlags::kF2));
    EXPECT_TRUE(flags.Has(TestFlags::kF3));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF1));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF2));
    EXPECT_FALSE(flags.HasOnly(TestFlags::kF3));
}

TEST(Flags, SetAll) {
    Util::Flags<TestFlags> flags;
    flags.SetAll(true);
    EXPECT_TRUE(flags.Has(TestFlags::kF1));
    EXPECT_TRUE(flags.Has(TestFlags::kF2));
    EXPECT_TRUE(flags.Has(TestFlags::kF3));
    flags.SetAll(false);
    EXPECT_FALSE(flags.Has(TestFlags::kF1));
    EXPECT_FALSE(flags.Has(TestFlags::kF2));
    EXPECT_FALSE(flags.Has(TestFlags::kF3));
}

TEST(Flags, FlagsReset) {
    Util::Flags<TestFlags> flags;
    EXPECT_FALSE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestFlags::kF1));
    flags.Set(TestFlags::kF1);
    flags.Set(TestFlags::kF2);
    flags.Set(TestFlags::kF3);
    EXPECT_TRUE(flags.Has(TestFlags::kF1));
    EXPECT_TRUE(flags.Has(TestFlags::kF2));
    EXPECT_TRUE(flags.Has(TestFlags::kF3));
    flags.Reset(TestFlags::kF2);
    EXPECT_TRUE(flags.Has(TestFlags::kF1));
    EXPECT_FALSE(flags.Has(TestFlags::kF2));
    EXPECT_TRUE(flags.Has(TestFlags::kF3));
}

TEST(Flags, FlagsHasAnyFrom) {
    Util::Flags<TestFlags> flags1;
    Util::Flags<TestFlags> flags2;
    EXPECT_FALSE(flags1.HasAnyFrom(flags2));

    flags1.Set(TestFlags::kF2);
    flags1.Set(TestFlags::kF3);
    EXPECT_FALSE(flags1.HasAnyFrom(flags2));

    flags2.Set(TestFlags::kF1);
    EXPECT_FALSE(flags1.HasAnyFrom(flags2));

    flags2.Set(TestFlags::kF2);
    EXPECT_TRUE(flags1.HasAnyFrom(flags2));

    flags2.Set(TestFlags::kF3);
    EXPECT_TRUE(flags1.HasAnyFrom(flags2));
}

TEST(Flags, ToFromString) {
    Util::Flags<TestFlags> flags;
    EXPECT_EQ("", flags.ToString());
    flags.Set(TestFlags::kF3);
    EXPECT_EQ("kF3", flags.ToString());
    flags.Set(TestFlags::kF1);
    EXPECT_EQ("kF1|kF3", flags.ToString());
    Util::Flags<TestFlags> flags2;
    EXPECT_TRUE(Util::Flags<TestFlags>::FromString("kF3|kF1", flags2));
    EXPECT_TRUE(flags2.Has(TestFlags::kF1));
    EXPECT_FALSE(flags2.Has(TestFlags::kF2));
    EXPECT_TRUE(flags2.Has(TestFlags::kF3));
}

TEST(Flags, MoreThanMax) {
    // magic_enum has a maximum value of 128 for enum values. This is normally
    // not a problem, but for bit-shifted flags it can be. This tests that
    // case.
    enum class BigFlags : uint32_t {
        kF1  = (1 << 0),
        kF2  = (1 << 1),
        kF3  = (1 << 2),
        kF4  = (1 << 3),
        kF5  = (1 << 4),
        kF6  = (1 << 5),
        kF7  = (1 << 6),
        kF8  = (1 << 7),
        kF9  = (1 << 8),  // Too big!
        kF10 = (1 << 9),  // Also too big!
    };

    Util::Flags<BigFlags> flags;
    flags.SetAll(true);
    EXPECT_EQ("kF1|kF2|kF3|kF4|kF5|kF6|kF7|kF8|kF9|kF10", flags.ToString());
}

}  // anonymous namespace
