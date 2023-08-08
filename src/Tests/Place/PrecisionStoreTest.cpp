#include "Place/PrecisionStore.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class PrecisionStoreTest : public TestBase {};

TEST_F(PrecisionStoreTest, PrecisionStore) {
    PrecisionStore ps;
    EXPECT_EQ(1,  ps.GetLinearPrecision());
    EXPECT_EQ(15, ps.GetAngularPrecision());
}

TEST_F(PrecisionStoreTest, Linear) {
    PrecisionStore ps;
    EXPECT_EQ(12,                 ps.Apply(11.75f));
    EXPECT_EQ(Vector3f(2, 3, -4), ps.Apply(Vector3f(1.9f, 3.2f, -3.9f)));
    EXPECT_EQ(Point3f(2, 3, -4),  ps.Apply(Point3f(1.9f, 3.2f, -3.9f)));

    EXPECT_EQ(1,                  ps.ApplyPositive(-3.1f));
    EXPECT_EQ(Vector3f(2, 3, 1),  ps.ApplyPositive(Vector3f(1.9f, 3.2f, -3.9f)));
    EXPECT_EQ(Point3f(2, 3, 1),   ps.ApplyPositive(Point3f(1.9f, 3.2f, -3.9f)));

    float distance;
    EXPECT_TRUE(ps.AreClose(Point3f(1, 2, 3), Point3f(1.75f, 2, 3), distance));
    EXPECT_EQ(.75f, distance);
    EXPECT_FALSE(ps.AreClose(Point3f(1, 2, 3), Point3f(1, 2, 4.5f), distance));
    EXPECT_EQ(1.5f, distance);
}

TEST_F(PrecisionStoreTest, Angular) {
    // Shorthand
    const auto ang = [](float deg){ return Anglef::FromDegrees(deg); };

    PrecisionStore ps;
    EXPECT_EQ(ang(15), ps.ApplyAngle(ang(11.75f)));

    Anglef difference;
    EXPECT_TRUE(ps.AreAnglesClose(ang(-4), ang(10), difference));
    EXPECT_CLOSE(14, difference.Degrees());
    EXPECT_FALSE(ps.AreAnglesClose(ang(100), ang(115.4f), difference));
    EXPECT_CLOSE(15.4f, difference.Degrees());
}

TEST_F(PrecisionStoreTest, IncreaseDecrease) {
    PrecisionStore ps;
    EXPECT_TRUE(ps.CanIncrease());
    EXPECT_FALSE(ps.CanDecrease());

    EXPECT_TRUE(ps.Increase());
    EXPECT_TRUE(ps.CanIncrease());
    EXPECT_TRUE(ps.CanDecrease());
    EXPECT_EQ(.1f, ps.GetLinearPrecision());
    EXPECT_EQ(5,   ps.GetAngularPrecision());

    EXPECT_TRUE(ps.Increase());
    EXPECT_FALSE(ps.CanIncrease());
    EXPECT_TRUE(ps.CanDecrease());
    EXPECT_EQ(.01f, ps.GetLinearPrecision());
    EXPECT_EQ(1,    ps.GetAngularPrecision());

    EXPECT_FALSE(ps.Increase());
    EXPECT_FALSE(ps.CanIncrease());
    EXPECT_TRUE(ps.CanDecrease());
    EXPECT_EQ(.01f, ps.GetLinearPrecision());
    EXPECT_EQ(1,    ps.GetAngularPrecision());

    EXPECT_TRUE(ps.Decrease());
    EXPECT_TRUE(ps.CanIncrease());
    EXPECT_TRUE(ps.CanDecrease());
    EXPECT_EQ(.1f, ps.GetLinearPrecision());
    EXPECT_EQ(5,   ps.GetAngularPrecision());

    EXPECT_TRUE(ps.Decrease());
    EXPECT_TRUE(ps.CanIncrease());
    EXPECT_FALSE(ps.CanDecrease());
    EXPECT_EQ(1,  ps.GetLinearPrecision());
    EXPECT_EQ(15, ps.GetAngularPrecision());

    EXPECT_FALSE(ps.Decrease());
    EXPECT_TRUE(ps.CanIncrease());
    EXPECT_FALSE(ps.CanDecrease());
    EXPECT_EQ(1,  ps.GetLinearPrecision());
    EXPECT_EQ(15, ps.GetAngularPrecision());
}

TEST_F(PrecisionStoreTest, Changes) {
    size_t change_count = 0;

    PrecisionStore ps;
    ps.GetChanged().AddObserver("key", [&](){ ++change_count; });

    EXPECT_EQ(0U, change_count);

    ps.Increase();
    EXPECT_EQ(1U, change_count);
    ps.Increase();
    EXPECT_EQ(2U, change_count);
    ps.Increase();  // No effect.
    EXPECT_EQ(2U, change_count);

    ps.Decrease();
    EXPECT_EQ(3U, change_count);
    ps.Decrease();
    EXPECT_EQ(4U, change_count);
    ps.Decrease();  // No effect.
    EXPECT_EQ(4U, change_count);
}
