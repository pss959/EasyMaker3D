#include <ion/math/vectorutils.h>

#include "Math/Polygon.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class PolygonTest : public TestBase {};

TEST_F(PolygonTest, Triangle) {
    const std::vector<Point2f> points{
        Point2f(0, 0),
        Point2f(1, 0),
        Point2f(0, 1),
    };
    const Polygon poly(points);

    EXPECT_EQ(3U, poly.GetPoints().size());
    EXPECT_EQ(Point2f(0, 0), poly.GetPoints()[0]);
    EXPECT_EQ(Point2f(1, 0), poly.GetPoints()[1]);
    EXPECT_EQ(Point2f(0, 1), poly.GetPoints()[2]);

    EXPECT_EQ(1U, poly.GetBorderCounts().size());
    EXPECT_EQ(3U, poly.GetBorderCounts()[0]);

    EXPECT_EQ(3U, poly.GetOuterBorderPoints().size());
    EXPECT_EQ(Point2f(0, 0), poly.GetOuterBorderPoints()[0]);
    EXPECT_EQ(Point2f(1, 0), poly.GetOuterBorderPoints()[1]);
    EXPECT_EQ(Point2f(0, 1), poly.GetOuterBorderPoints()[2]);

    EXPECT_EQ(0U, poly.GetHoleCount());

    EXPECT_EQ(Range2f(Point2f(0, 0), Point2f(1, 1)), poly.GetBoundingRect());

    // Pass empty border counts; should be the same.
    const Polygon poly2(points, std::vector<size_t>());
    EXPECT_EQ(poly.GetPoints(),       poly2.GetPoints());
    EXPECT_EQ(poly.GetBorderCounts(), poly2.GetBorderCounts());
}

TEST_F(PolygonTest, Dups) {
    // Test that duplicate adjacent points are cleaned up.
    const std::vector<Point2f> points{
        Point2f(-3, -5),
        Point2f(-3, -5),
        Point2f( 2, -5),
        Point2f( 6,  5),
        Point2f( 6,  5),
        Point2f( 3,  5),
        Point2f( 3,  5),
        Point2f(-3, -5),
    };
    const Polygon poly(points);
    EXPECT_EQ(4U, poly.GetPoints().size());
    EXPECT_EQ(Point2f(-3, -5), poly.GetPoints()[0]);
    EXPECT_EQ(Point2f( 2, -5), poly.GetPoints()[1]);
    EXPECT_EQ(Point2f( 6,  5), poly.GetPoints()[2]);
    EXPECT_EQ(Point2f( 3,  5), poly.GetPoints()[3]);
}

TEST_F(PolygonTest, Hole) {
    const std::vector<Point2f> points{
        // Outer border.
        Point2f(-3, -5),
        Point2f( 3, -5),
        Point2f( 3,  5),
        Point2f(-3,  5),
        // Hole (clockwise).
        Point2f(-1, -3),
        Point2f(-1,  3),
        Point2f( 1,  3),
        Point2f( 1, -3),
    };
    const std::vector<size_t> border_counts{ 4, 4 };
    const Polygon poly(points, border_counts);

    EXPECT_EQ(8U, poly.GetPoints().size());
    EXPECT_EQ(points[0], poly.GetPoints()[0]);
    EXPECT_EQ(points[1], poly.GetPoints()[1]);
    EXPECT_EQ(points[2], poly.GetPoints()[2]);
    EXPECT_EQ(points[3], poly.GetPoints()[3]);
    EXPECT_EQ(points[4], poly.GetPoints()[4]);
    EXPECT_EQ(points[5], poly.GetPoints()[5]);
    EXPECT_EQ(points[6], poly.GetPoints()[6]);
    EXPECT_EQ(points[7], poly.GetPoints()[7]);

    EXPECT_EQ(2U, poly.GetBorderCounts().size());
    EXPECT_EQ(4U, poly.GetBorderCounts()[0]);
    EXPECT_EQ(4U, poly.GetBorderCounts()[1]);

    EXPECT_EQ(4U, poly.GetOuterBorderPoints().size());
    EXPECT_EQ(points[0], poly.GetOuterBorderPoints()[0]);
    EXPECT_EQ(points[1], poly.GetOuterBorderPoints()[1]);
    EXPECT_EQ(points[2], poly.GetOuterBorderPoints()[2]);
    EXPECT_EQ(points[3], poly.GetOuterBorderPoints()[3]);

    EXPECT_EQ(1U, poly.GetHoleCount());
    EXPECT_EQ(4U, poly.GetHolePoints(0).size());
    EXPECT_EQ(points[4], poly.GetHolePoints(0)[0]);
    EXPECT_EQ(points[5], poly.GetHolePoints(0)[1]);
    EXPECT_EQ(points[6], poly.GetHolePoints(0)[2]);
    EXPECT_EQ(points[7], poly.GetHolePoints(0)[3]);

    EXPECT_EQ(Range2f(Point2f(-3, -5), Point2f(3, 5)), poly.GetBoundingRect());
}

TEST_F(PolygonTest, TwoHoles) {
    const std::vector<Point2f> points{
        // Outer border.
        Point2f(-3, -5),
        Point2f( 3, -5),
        Point2f( 3,  5),
        Point2f(-3,  5),
        // Top Hole (clockwise).
        Point2f(-1,  1),
        Point2f(-1,  3),
        Point2f( 1,  3),
        Point2f( 1,  1),
        // Bottom Hole (clockwise).
        Point2f(-1, -3),
        Point2f(-1, -1),
        Point2f( 1, -1),
        Point2f( 1, -3),
    };
    const std::vector<size_t> border_counts{ 4, 4, 4 };
    const Polygon poly(points, border_counts);

    EXPECT_EQ(12U,        poly.GetPoints().size());
    EXPECT_EQ(points[0],  poly.GetPoints()[0]);
    EXPECT_EQ(points[1],  poly.GetPoints()[1]);
    EXPECT_EQ(points[2],  poly.GetPoints()[2]);
    EXPECT_EQ(points[3],  poly.GetPoints()[3]);
    EXPECT_EQ(points[4],  poly.GetPoints()[4]);
    EXPECT_EQ(points[5],  poly.GetPoints()[5]);
    EXPECT_EQ(points[6],  poly.GetPoints()[6]);
    EXPECT_EQ(points[7],  poly.GetPoints()[7]);
    EXPECT_EQ(points[8],  poly.GetPoints()[8]);
    EXPECT_EQ(points[9],  poly.GetPoints()[9]);
    EXPECT_EQ(points[10], poly.GetPoints()[10]);
    EXPECT_EQ(points[11], poly.GetPoints()[11]);

    EXPECT_EQ(3U, poly.GetBorderCounts().size());
    EXPECT_EQ(4U, poly.GetBorderCounts()[0]);
    EXPECT_EQ(4U, poly.GetBorderCounts()[1]);
    EXPECT_EQ(4U, poly.GetBorderCounts()[2]);

    EXPECT_EQ(4U,        poly.GetOuterBorderPoints().size());
    EXPECT_EQ(points[0], poly.GetOuterBorderPoints()[0]);
    EXPECT_EQ(points[1], poly.GetOuterBorderPoints()[1]);
    EXPECT_EQ(points[2], poly.GetOuterBorderPoints()[2]);
    EXPECT_EQ(points[3], poly.GetOuterBorderPoints()[3]);

    EXPECT_EQ(2U,         poly.GetHoleCount());
    EXPECT_EQ(4U,         poly.GetHolePoints(0).size());
    EXPECT_EQ(points[4],  poly.GetHolePoints(0)[0]);
    EXPECT_EQ(points[5],  poly.GetHolePoints(0)[1]);
    EXPECT_EQ(points[6],  poly.GetHolePoints(0)[2]);
    EXPECT_EQ(points[7],  poly.GetHolePoints(0)[3]);
    EXPECT_EQ(4U,         poly.GetHolePoints(1).size());
    EXPECT_EQ(points[8],  poly.GetHolePoints(1)[0]);
    EXPECT_EQ(points[9],  poly.GetHolePoints(1)[1]);
    EXPECT_EQ(points[10], poly.GetHolePoints(1)[2]);
    EXPECT_EQ(points[11], poly.GetHolePoints(1)[3]);

    EXPECT_EQ(Range2f(Point2f(-3, -5), Point2f(3, 5)), poly.GetBoundingRect());
}

TEST_F(PolygonTest, Scale) {
    const std::vector<Point2f> points{
        Point2f(1, 2),
        Point2f(2, 3),
        Point2f(0, 4),
    };
    Polygon poly(points);
    poly.Scale(3);
    EXPECT_EQ(Point2f(3,  6), poly.GetPoints()[0]);
    EXPECT_EQ(Point2f(6,  9), poly.GetPoints()[1]);
    EXPECT_EQ(Point2f(0, 12), poly.GetPoints()[2]);
}

TEST_F(PolygonTest, Translate) {
    const std::vector<Point2f> points{
        Point2f(1, 2),
        Point2f(2, 3),
        Point2f(0, 4),
    };
    Polygon poly(points);
    poly.Translate(Vector2f(100, 200));
    EXPECT_EQ(Point2f(101, 202), poly.GetPoints()[0]);
    EXPECT_EQ(Point2f(102, 203), poly.GetPoints()[1]);
    EXPECT_EQ(Point2f(100, 204), poly.GetPoints()[2]);
}
