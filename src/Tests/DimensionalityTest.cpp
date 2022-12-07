#include "Base/Dimensionality.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"

TEST(Dimensionality, DefaultConstructor) {
    Dimensionality d;
    EXPECT_EQ(0, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::X));
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::Y));
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::Z));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_FALSE(d.HasDimension(1));
    EXPECT_FALSE(d.HasDimension(2));
}

TEST(Dimensionality, AddDimensionAndClear) {
    Dimensionality d;
    d.AddDimension(1);
    EXPECT_EQ(1, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::X));
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::Y));
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::Z));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_FALSE(d.HasDimension(2));
    d.AddDimension(2);
    EXPECT_EQ(2, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::X));
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::Y));
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::Z));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_TRUE(d.HasDimension(2));
    d.AddDimension(2);  // No effect.
    EXPECT_EQ(2, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::X));
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::Y));
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::Z));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_TRUE(d.HasDimension(2));
    d.AddDimension(0);
    EXPECT_EQ(3, d.GetCount());
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::X));
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::Y));
    EXPECT_TRUE(d.HasDimension(Dimensionality::Dim::Z));
    EXPECT_TRUE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_TRUE(d.HasDimension(2));

    d.Clear();
    EXPECT_EQ(0, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::X));
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::Y));
    EXPECT_FALSE(d.HasDimension(Dimensionality::Dim::Z));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_FALSE(d.HasDimension(1));
    EXPECT_FALSE(d.HasDimension(2));
}

TEST(Dimensionality, GetIndex) {
    Dimensionality d;
    EXPECT_EQ(-1, d.GetIndex());

    d.AddDimension(0);
    EXPECT_EQ(0, d.GetIndex());

    d.Clear();
    d.AddDimension(1);
    EXPECT_EQ(1, d.GetIndex());

    d.Clear();
    d.AddDimension(2);
    EXPECT_EQ(2, d.GetIndex());

    d.AddDimension(1);
    EXPECT_EQ(-1, d.GetIndex());
}

TEST(Dimensionality, StringConstructor) {
    {
        Dimensionality d("");
        EXPECT_EQ(0, d.GetCount());
        EXPECT_FALSE(d.HasDimension(0));
        EXPECT_FALSE(d.HasDimension(1));
        EXPECT_FALSE(d.HasDimension(2));
    }
    {
        Dimensionality d("X");
        EXPECT_EQ(1, d.GetCount());
        EXPECT_TRUE(d.HasDimension(0));
        EXPECT_FALSE(d.HasDimension(1));
        EXPECT_FALSE(d.HasDimension(2));
    }
    {
        Dimensionality d("ZX");
        EXPECT_EQ(2, d.GetCount());
        EXPECT_TRUE(d.HasDimension(0));
        EXPECT_FALSE(d.HasDimension(1));
        EXPECT_TRUE(d.HasDimension(2));
    }
    {
        Dimensionality d("ZXYZZYZXYYXZ");
        EXPECT_EQ(3, d.GetCount());
        EXPECT_TRUE(d.HasDimension(0));
        EXPECT_TRUE(d.HasDimension(1));
        EXPECT_TRUE(d.HasDimension(2));
    }
    TEST_THROW(Dimensionality d("UV"), AssertException, "Invalid character");
}
