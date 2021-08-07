#include "Flags.h"
#include "Testing.h"

// Enum used for testing.
enum class TestEnum : uint32_t {
    kF1 = (1 << 0),
    kF2 = (1 << 1),
    kF3 = (1 << 2),
};

TEST(Flags, SetAndHas) {
    Flags<TestEnum> flags;
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

TEST(Flags, HasAnyFrom) {
    Flags<TestEnum> flags1;
    Flags<TestEnum> flags2;
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
