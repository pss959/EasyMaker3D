#include "Math/Dimensionality.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"

TEST(DimensionalityTest, DefaultConstructor) {
    Dimensionality d;
    EXPECT_EQ(0, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dim::kX));
    EXPECT_FALSE(d.HasDimension(Dim::kY));
    EXPECT_FALSE(d.HasDimension(Dim::kZ));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_FALSE(d.HasDimension(1));
    EXPECT_FALSE(d.HasDimension(2));
}

TEST(DimensionalityTest, DimConstructor) {
    Dimensionality dx(0);
    EXPECT_EQ(1, dx.GetCount());
    EXPECT_TRUE(dx.HasDimension(Dim::kX));
    EXPECT_FALSE(dx.HasDimension(Dim::kY));
    EXPECT_FALSE(dx.HasDimension(Dim::kZ));
    EXPECT_TRUE(dx.HasDimension(0));
    EXPECT_FALSE(dx.HasDimension(1));
    EXPECT_FALSE(dx.HasDimension(2));

    Dimensionality dy(1);
    EXPECT_EQ(1, dy.GetCount());
    EXPECT_FALSE(dy.HasDimension(Dim::kX));
    EXPECT_TRUE(dy.HasDimension(Dim::kY));
    EXPECT_FALSE(dy.HasDimension(Dim::kZ));
    EXPECT_FALSE(dy.HasDimension(0));
    EXPECT_TRUE(dy.HasDimension(1));
    EXPECT_FALSE(dy.HasDimension(2));

    Dimensionality dz(2);
    EXPECT_EQ(1, dz.GetCount());
    EXPECT_FALSE(dz.HasDimension(Dim::kX));
    EXPECT_FALSE(dz.HasDimension(Dim::kY));
    EXPECT_TRUE(dz.HasDimension(Dim::kZ));
    EXPECT_FALSE(dz.HasDimension(0));
    EXPECT_FALSE(dz.HasDimension(1));
    EXPECT_TRUE(dz.HasDimension(2));
}

TEST(DimensionalityTest, AddDimensionAndClear) {
    Dimensionality d;
    d.AddDimension(1);
    EXPECT_EQ(1, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dim::kX));
    EXPECT_TRUE(d.HasDimension(Dim::kY));
    EXPECT_FALSE(d.HasDimension(Dim::kZ));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_FALSE(d.HasDimension(2));
    d.AddDimension(2);
    EXPECT_EQ(2, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dim::kX));
    EXPECT_TRUE(d.HasDimension(Dim::kY));
    EXPECT_TRUE(d.HasDimension(Dim::kZ));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_TRUE(d.HasDimension(2));
    d.AddDimension(2);  // No effect.
    EXPECT_EQ(2, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dim::kX));
    EXPECT_TRUE(d.HasDimension(Dim::kY));
    EXPECT_TRUE(d.HasDimension(Dim::kZ));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_TRUE(d.HasDimension(2));
    d.AddDimension(0);
    EXPECT_EQ(3, d.GetCount());
    EXPECT_TRUE(d.HasDimension(Dim::kX));
    EXPECT_TRUE(d.HasDimension(Dim::kY));
    EXPECT_TRUE(d.HasDimension(Dim::kZ));
    EXPECT_TRUE(d.HasDimension(0));
    EXPECT_TRUE(d.HasDimension(1));
    EXPECT_TRUE(d.HasDimension(2));

    d.Clear();
    EXPECT_EQ(0, d.GetCount());
    EXPECT_FALSE(d.HasDimension(Dim::kX));
    EXPECT_FALSE(d.HasDimension(Dim::kY));
    EXPECT_FALSE(d.HasDimension(Dim::kZ));
    EXPECT_FALSE(d.HasDimension(0));
    EXPECT_FALSE(d.HasDimension(1));
    EXPECT_FALSE(d.HasDimension(2));
}

TEST(DimensionalityTest, GetIndex) {
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

TEST(DimensionalityTest, StringConstructor) {
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

TEST(DimensionalityTest, GetAsString) {
    EXPECT_EQ("",    Dimensionality("").GetAsString());
    EXPECT_EQ("X",   Dimensionality("X").GetAsString());
    EXPECT_EQ("Y",   Dimensionality("Y").GetAsString());
    EXPECT_EQ("Z",   Dimensionality("Z").GetAsString());
    EXPECT_EQ("XY",  Dimensionality("XY").GetAsString());
    EXPECT_EQ("XY",  Dimensionality("YX").GetAsString());
    EXPECT_EQ("XYZ", Dimensionality("XYZ").GetAsString());
    EXPECT_EQ("XYZ", Dimensionality("YZX").GetAsString());
}
