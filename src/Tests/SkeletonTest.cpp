#include "Math/Polygon.h"
#include "Math/Skeleton2D.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class SkeletonTest : public TestBase {
  protected:
    // Returns the index of the skeleton vertex with the given point and
    // source_index.
    static size_t FindIndex(const Skeleton2D &skel, const Point2f &point,
                            int source_index) {
        for (size_t i = 0; i < skel.GetVertices().size(); ++i) {
            const auto &v = skel.GetVertices()[i];
            if (v.point == point && v.source_index == source_index)
                return i;
        }
        EXPECT_TRUE(false);
        return 0;
    }

    // Returns true if an edge with the given endpoints and bisector indices is
    // found in the skeleton edges.
    static bool HasEdge(const Skeleton2D &skel,
                        size_t v0, size_t v1, int bisected0, int bisected1) {
        for (const auto &e: skel.GetEdges())
            if (e.v0_index == v0 &&
                e.v1_index == v1 &&
                e.bisected_index0 == bisected0 &&
                e.bisected_index1 == bisected1)
                return true;
        return false;
    }

    // Dumps a skeleton for debugging.
    template <typename T>
    static void DumpSkeleton(const Skeleton<T> &skel) {
        for (const auto &v: skel.GetVertices())
            std::cerr << "Vertex @ " << v.point << " source = "
                      << v.source_index << "\n";
        for (const auto &e: skel.GetEdges())
            std::cerr << "Edge from " << e.v0_index << " to " << e.v1_index
                      << " bisects "  << e.bisected_index0
                      << " and "      << e.bisected_index1 << "\n";
    }
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

    // 4 original points + center point for bisectors.
    EXPECT_EQ(5U, skel.GetVertices().size());

    // Get indices for original point and center point.
    std::vector<size_t> indices;
    for (size_t i = 0; i < points.size(); ++i)
        indices.push_back(FindIndex(skel, points[i], i));
    indices.push_back(FindIndex(skel, Point2f(.5f, .5f), -1));

    // 4 bisectors.
    EXPECT_EQ(4U, skel.GetEdges().size());
    EXPECT_TRUE(HasEdge(skel, indices[0], indices[4], 1, 3));
    EXPECT_TRUE(HasEdge(skel, indices[1], indices[4], 2, 0));
    EXPECT_TRUE(HasEdge(skel, indices[2], indices[4], 3, 1));
    EXPECT_TRUE(HasEdge(skel, indices[3], indices[4], 0, 2));
}

TEST_F(SkeletonTest, Skeleton2DRect) {
    const std::vector<Point2f> points{
        Point2f(0, 0),
        Point2f(1, 0),
        Point2f(1, 2),
        Point2f(0, 2),
    };
    Polygon poly(points);

    Skeleton2D skel;
    skel.BuildForPolygon(poly);

    // 4 original points + 2 points for center bisector.
    EXPECT_EQ(6U, skel.GetVertices().size());

    // Get indices for original point and center point.
    std::vector<size_t> indices;
    for (size_t i = 0; i < points.size(); ++i)
        indices.push_back(FindIndex(skel, points[i], i));
    indices.push_back(FindIndex(skel, Point2f(.5f,  .5f), -1));
    indices.push_back(FindIndex(skel, Point2f(.5f, 1.5f), -1));

    // 4 bisectors + 1 internal edge.
    EXPECT_EQ(5U, skel.GetEdges().size());
    EXPECT_TRUE(HasEdge(skel, indices[0], indices[4], 1, 3));
    EXPECT_TRUE(HasEdge(skel, indices[1], indices[4], 2, 0));
    EXPECT_TRUE(HasEdge(skel, indices[2], indices[5], 3, 1));
    EXPECT_TRUE(HasEdge(skel, indices[3], indices[5], 0, 2));
    EXPECT_TRUE(HasEdge(skel, indices[4], indices[5], -1, -1));
}
