#include "Math/Bevel.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class BevelTest : public TestBase {};

TEST_F(BevelTest, DefaultBevel) {
    const Bevel b;
    EXPECT_EQ(Bevel::CreateDefaultProfile(), b.profile);
    EXPECT_EQ(1,                             b.scale);
    EXPECT_EQ(Anglef::FromDegrees(120),      b.max_angle);
}

TEST_F(BevelTest, Equality) {
    Bevel b0;
    Bevel b1;
    Bevel b2;
    Bevel b3;

    // Default bevels are equal.
    EXPECT_EQ(b0, b1);

    b1.profile.AppendPoint(Point2f(.2f, .2f));
    EXPECT_NE(b0, b1);

    b2.scale = 1.2f;
    EXPECT_NE(b0, b2);
    EXPECT_NE(b1, b2);

    b3.max_angle = Anglef::FromDegrees(100);
    EXPECT_NE(b0, b3);
    EXPECT_NE(b1, b3);
    EXPECT_NE(b2, b3);
}
