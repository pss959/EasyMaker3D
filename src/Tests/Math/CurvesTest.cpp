#include <algorithm>

#include <ion/math/angleutils.h>

#include "Math/Curves.h"
#include "Math/Linear.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CurvesTest : public TestBase {
  protected:
    /// Convenience to create a CircleArc with start and arc angles in degrees.
    static CircleArc GetArc_(float start_deg, float arc_deg) {
        return CircleArc(Anglef::FromDegrees(start_deg),
                         Anglef::FromDegrees(arc_deg));
    }
};

TEST_F(CurvesTest, CircleArc) {
    CircleArc arc;
    EXPECT_EQ(Anglef::FromDegrees(0),   arc.start_angle);
    EXPECT_EQ(Anglef::FromDegrees(360), arc.arc_angle);

    CircleArc arc2 = GetArc_(40, -120);
    EXPECT_EQ(Anglef::FromDegrees(40),   arc2.start_angle);
    EXPECT_EQ(Anglef::FromDegrees(-120), arc2.arc_angle);

    // Test equality operator.
    EXPECT_EQ(arc,  arc);
    EXPECT_EQ(arc2, arc2);
    EXPECT_NE(arc,  arc2);
    EXPECT_NE(arc2, arc);
}

TEST_F(CurvesTest, GetCirclePoints) {
    using ion::math::Length;

    // No points.
    auto pts = GetCirclePoints(0, 0, true);
    EXPECT_EQ(0U, pts.size());

    // 10 points with radius 0.
    pts = GetCirclePoints(10, 0, true);
    EXPECT_EQ(10U, pts.size());
    EXPECT_TRUE(std::all_of(
                    pts.begin(), pts.end(),
                    [](const Point2f &p){ return p == Point2f::Zero(); }));

    // 20 points with radius 10.
    pts = GetCirclePoints(20, 10, true);
    EXPECT_EQ(20U, pts.size());
    EXPECT_TRUE(std::all_of(
                    pts.begin(), pts.end(),
                    [](const Point2f &p){
                        return AreClose(10, Length(p - Point2f::Zero()));
                    }));

    // Same, in reverse direction.
    auto pts2 = GetCirclePoints(20, 10, false);
    EXPECT_EQ(20U, pts2.size());
    for (int i = 0; i < 20; ++i) {
        // Should be the same point with Y negated.
        EXPECT_EQ(Point2f(pts[i][0], -pts[i][1]), pts2[i]);
    }
}

TEST_F(CurvesTest, GetCircleArcPoints) {
    using ion::math::Length;

    // No points.
    auto pts = GetCircleArcPoints(0, 0, CircleArc());
    EXPECT_EQ(0U, pts.size());

    // 10 points with radius 0 on 90 degree arc.
    pts = GetCircleArcPoints(10, 0, GetArc_(30, 90));
    EXPECT_EQ(10U, pts.size());
    EXPECT_TRUE(std::all_of(
                    pts.begin(), pts.end(),
                    [](const Point2f &p){ return p == Point2f::Zero(); }));

    // 20 points with radius 10 on 90 degree arc.
    pts = GetCircleArcPoints(20, 10, GetArc_(30, 90));
    EXPECT_EQ(20U, pts.size());
    EXPECT_TRUE(std::all_of(
                    pts.begin(), pts.end(),
                    [](const Point2f &p){
                        return AreClose(10, Length(p - Point2f::Zero()));
                    }));
}
