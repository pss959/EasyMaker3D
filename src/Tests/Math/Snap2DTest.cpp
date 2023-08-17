#include "Math/Snap2D.h"
#include "Math/ToString.h"
#include "Math/Types.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class Snap2DTest : public TestBase {};

TEST_F(Snap2DTest, ToleranceAngle) {
    Snap2D s;
    EXPECT_EQ(Anglef::FromDegrees(0), s.GetToleranceAngle());
    s.SetToleranceAngle(Anglef::FromDegrees(15));
    EXPECT_EQ(Anglef::FromDegrees(15), s.GetToleranceAngle());
}

TEST_F(Snap2DTest, SnapPointBetween) {
    Snap2D s;
    s.SetToleranceAngle(Anglef::FromDegrees(10));

    // Tests snapping to a diagonal from (0,0) to (1,1).
    const auto test_diag = [&](const Str &desc,
                               float x, float y, Snap2D::Result r,
                               float exp_x, float exp_y){
        SCOPED_TRACE("DIAG " + desc);
        Point2f p(x, y);
        EXPECT_ENUM_EQ(r, s.SnapPointBetween(Point2f(0, 0), Point2f(1, 1), p));
        EXPECT_EQ(Point2f(exp_x, exp_y), p);
    };

    // Point too far away.
    test_diag("far", .6f, .3f, Snap2D::Result::kNeither, .6f, .3f);

    // Horizontal to p0.
    test_diag("horizp0", .6f, .1f, Snap2D::Result::kPoint0, .6f, 0);

    // Vertical to p0.
    test_diag("vertp0", .1f, .8f, Snap2D::Result::kPoint0, 0, .8f);

    // Horizontal to p1.
    test_diag("horizp1", .6f, .95f, Snap2D::Result::kPoint1, .6f, 1);

    // Vertical to p1.
    test_diag("vertp1", .9f, .3f, Snap2D::Result::kPoint1, 1, .3f);

    // Point close to diagonal.
    test_diag("diag", .51f, .49f, Snap2D::Result::kBoth, .5f, .5f);

    // Tests snapping to a vertical line from (0,0) to (0,1).
    const auto test_vert = [&](const Str &desc,
                               float x, float y, Snap2D::Result r,
                               float exp_x, float exp_y){
        Point2f p(x, y);
        SCOPED_TRACE("VERT " + desc);
        EXPECT_ENUM_EQ(r, s.SnapPointBetween(Point2f(0, 0), Point2f(0, 1), p));
        EXPECT_PTS_CLOSE2(Point2f(exp_x, exp_y), p);
    };

    // Point too far away.
    test_vert("far", .3f, .8f, Snap2D::Result::kNeither, .3f, .8f);

    // Horizontal to p0.
    test_vert("horizp0", .6f, .1f, Snap2D::Result::kPoint0, .6f, 0);

    // Horizontal to p1.
    test_vert("horizp1", .6f, .95f, Snap2D::Result::kPoint1, .6f, 1);

    // Diagonal from p0.
    test_vert("diagp0", .31f, .29f, Snap2D::Result::kPoint0, .30017f, .30017f);

    // Diagonal from p1.
    test_vert("diagp1", .31f, .71f, Snap2D::Result::kPoint1, .30017f, .6999f);

    // Diagonal from both.
    test_vert("diagboth", .51f, .49f, Snap2D::Result::kBoth, .5f, .5f);

    // Points too close together.
    Point2f p(.5f, .5f);
    EXPECT_ENUM_EQ(Snap2D::Result::kNeither,
                   s.SnapPointBetween(Point2f(.49999f, .49999f),
                                      Point2f(.50001f, .50001f), p));
    EXPECT_EQ(Point2f(.5f, .5f), p);
}
