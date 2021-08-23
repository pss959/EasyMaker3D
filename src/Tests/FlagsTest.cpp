#include "Testing.h"
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

    flags.Set(TestFlags::kF2);
    EXPECT_TRUE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestFlags::kF1));
    EXPECT_TRUE(flags.Has(TestFlags::kF2));
    EXPECT_FALSE(flags.Has(TestFlags::kF3));

    flags.Set(TestFlags::kF3);
    EXPECT_TRUE(flags.HasAny());
    EXPECT_FALSE(flags.Has(TestFlags::kF1));
    EXPECT_TRUE(flags.Has(TestFlags::kF2));
    EXPECT_TRUE(flags.Has(TestFlags::kF3));
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

}  // anonymous namespace
