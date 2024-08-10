#include "Math/Taper.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class TaperTest : public TestBase {};

TEST_F(TaperTest, DefaultTaper) {
    const Taper t;
    EXPECT_EQ(Dim::kY,                       t.axis);
    EXPECT_EQ(Taper::CreateDefaultProfile(), t.profile);
}

TEST_F(TaperTest, IsValidProfile) {
    // Default profile is valid.
    EXPECT_TRUE(Taper::IsValidProfile(Taper::CreateDefaultProfile()));

    // No points or 1 point..
    EXPECT_FALSE(Taper::IsValidProfile(Profile()));
    EXPECT_FALSE(Taper::IsValidProfile(
                     Profile(Profile::Type::kOpen, 1,
                             Profile::PointVec{ Point2f(0, 1) })));

    // Not an open profile.
    Profile::PointVec pts{ Point2f(0, 1), Point2f(1, 0) };
    EXPECT_FALSE(Taper::IsValidProfile(Profile::CreateFixedProfile(
                                           Point2f(0, 1), Point2f(1, 0), 2,
                                           Profile::PointVec())));
    EXPECT_FALSE(Taper::IsValidProfile(
                     Profile(Profile::Type::kClosed, 2, pts)));

    EXPECT_TRUE(Taper::IsValidProfile(Profile(Profile::Type::kOpen, 2, pts)));
    pts.insert(pts.begin() + 1, Point2f(.5f, .2f));
    EXPECT_TRUE(Taper::IsValidProfile(Profile(Profile::Type::kOpen, 3, pts)));

    // Wrong end points.
    pts[0].Set(0, .9f);
    EXPECT_FALSE(Taper::IsValidProfile(Profile(Profile::Type::kOpen, 2, pts)));
    pts[0].Set(0, 1);
    pts[2].Set(1, .1f);
    EXPECT_FALSE(Taper::IsValidProfile(Profile(Profile::Type::kOpen, 2, pts)));
    pts[2].Set(1, 0);

    // Points not monotonically decreasing in Y.
    pts.insert(pts.begin() + 2, Point2f(.5f, .25f));
    EXPECT_FALSE(Taper::IsValidProfile(Profile(Profile::Type::kOpen, 3, pts)));

    // Fix it.
    pts[2].Set(.5f, .1f);
    EXPECT_TRUE(Taper::IsValidProfile(Profile(Profile::Type::kOpen, 3, pts)));
}

TEST_F(TaperTest, Equality) {
    Taper t0;
    Taper t1;
    Taper t2;

    // Default tapers are equal.
    EXPECT_EQ(t0, t1);

    t1.axis = Dim::kZ;
    EXPECT_EQ(t1, t1);
    EXPECT_NE(t0, t1);

    t2.profile.AppendPoint(Point2f(.2f, .2f));
    EXPECT_EQ(t2, t2);
    EXPECT_NE(t0, t1);
    EXPECT_NE(t1, t2);
}
