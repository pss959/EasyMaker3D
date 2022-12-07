#include "Math/Polygon.h"
#include "Math/Skeleton2D.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class SkeletonTest : public TestBase {
};

TEST_F(SkeletonTest, Skeleton2DSquare) {
    const std::vector<Point2f> points{
        Point2f(0, 0),
        Point2f(1, 0),
        Point2f(1, 1),
        Point2f(0, 1),
    };
    Polygon poly(points);

    Skeleton2D skel;
    skel.BuildForPolygon(poly);
    EXPECT_EQ(5U, skel.GetVertices().size());
    // XXXX
}
