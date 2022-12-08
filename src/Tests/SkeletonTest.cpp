#include "Math/MeshBuilding.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Polygon.h"
#include "Math/Skeleton2D.h"
#include "Math/Skeleton3D.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class SkeletonTest : public TestBase {
  protected:
    // Returns the index of the skeleton vertex with the given point and
    // source_index.
    template <typename T>
    static size_t FindIndex(const Skeleton<T> &skel, const T &point,
                            int source_index) {
        for (size_t i = 0; i < skel.GetVertices().size(); ++i) {
            const auto &v = skel.GetVertices()[i];
            if (v.point == point && v.source_index == source_index)
                return i;
        }
        EXPECT_TRUE(false) << "Cannot find point " << point;
        return 0;
    }

    // Returns true if an edge with the given endpoints and bisector indices is
    // found in the skeleton edges.
    template <typename T>
    static bool HasEdge(const Skeleton<T> &skel,
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
        const auto &verts = skel.GetVertices();
        for (size_t i = 0; i < verts.size(); ++i)
            std::cerr << "Vertex [" << i << "] @ " << verts[i].point
                      << " source = " << verts[i].source_index << "\n";
        for (const auto &e: skel.GetEdges())
            std::cerr << "Edge from " << e.v0_index << " to " << e.v1_index
                      << " (source "  << verts[e.v0_index].source_index
                      << " to "       << verts[e.v1_index].source_index
                      << ") bisects " << e.bisected_index0
                      << " and "      << e.bisected_index1 << "\n";
    }

    // Dumps indices for debugging.
    static void DumpIndices(const std::vector<size_t> &indices) {
        for (size_t i = 0; i < indices.size(); ++i)
            std::cerr << " Index " << indices[i]
                      << " in position " << i << "\n";
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

    // Get indices for original points and center point.
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

TEST_F(SkeletonTest, Skeleton3DCube) {
    // 2x2x2 box centered on the origin.
    PolyMesh pbox(BuildBoxMesh(Vector3f(2, 2, 2)));
    MergeCoplanarFaces(pbox);

    Skeleton3D skel;
    skel.BuildForPolyMesh(pbox);

    // 8 original points + 6 center points for bisectors.
    EXPECT_EQ(14U, skel.GetVertices().size());

    // Get indices for original points and center points.
    std::vector<size_t> indices;
    for (size_t i = 0; i < pbox.vertices.size(); ++i)
        indices.push_back(FindIndex(skel, pbox.vertices[i]->point, i));
    indices.push_back(FindIndex(skel, Point3f(-1,  0,  0), -1));  // 6
    indices.push_back(FindIndex(skel, Point3f( 1,  0,  0), -1));  // 7
    indices.push_back(FindIndex(skel, Point3f( 0, -1,  0), -1));  // 8
    indices.push_back(FindIndex(skel, Point3f( 0,  1,  0), -1));  // 9
    indices.push_back(FindIndex(skel, Point3f( 0,  0, -1), -1));  // 10
    indices.push_back(FindIndex(skel, Point3f( 0,  0,  1), -1));  // 11

    // 4 bisectors per face.
    EXPECT_EQ(24U, skel.GetEdges().size());

    EXPECT_TRUE(HasEdge(skel, indices[0], indices[8], 1, 3));
    EXPECT_TRUE(HasEdge(skel, indices[1], indices[8], 2, 0));
    EXPECT_TRUE(HasEdge(skel, indices[3], indices[8], 3, 1));
    EXPECT_TRUE(HasEdge(skel, indices[2], indices[8], 0, 2));

    EXPECT_TRUE(HasEdge(skel, indices[2], indices[11], 2, 6));
    EXPECT_TRUE(HasEdge(skel, indices[3], indices[11], 5, 3));
    EXPECT_TRUE(HasEdge(skel, indices[7], indices[11], 6, 2));
    EXPECT_TRUE(HasEdge(skel, indices[6], indices[11], 3, 5));

    EXPECT_TRUE(HasEdge(skel, indices[4], indices[9], 6, 9));
    EXPECT_TRUE(HasEdge(skel, indices[6], indices[9], 5, 8));
    EXPECT_TRUE(HasEdge(skel, indices[7], indices[9], 9, 6));
    EXPECT_TRUE(HasEdge(skel, indices[5], indices[9], 8, 5));

    EXPECT_TRUE(HasEdge(skel, indices[1], indices[10], 0, 9));
    EXPECT_TRUE(HasEdge(skel, indices[0], indices[10], 8, 1));
    EXPECT_TRUE(HasEdge(skel, indices[4], indices[10], 9, 0));
    EXPECT_TRUE(HasEdge(skel, indices[5], indices[10], 1, 8));

    EXPECT_TRUE(HasEdge(skel, indices[3], indices[13], 1, 5));
    EXPECT_TRUE(HasEdge(skel, indices[1], indices[13], 9, 2));
    EXPECT_TRUE(HasEdge(skel, indices[5], indices[13], 5, 1));
    EXPECT_TRUE(HasEdge(skel, indices[7], indices[13], 2, 9));

    EXPECT_TRUE(HasEdge(skel, indices[4], indices[12], 0, 6));
    EXPECT_TRUE(HasEdge(skel, indices[0], indices[12], 3, 8));
    EXPECT_TRUE(HasEdge(skel, indices[2], indices[12], 6, 0));
    EXPECT_TRUE(HasEdge(skel, indices[6], indices[12], 8, 3));
}

TEST_F(SkeletonTest, Skeleton3DBox) {
    // 2x4x2 box centered on the origin.
    PolyMesh pbox(BuildBoxMesh(Vector3f(2, 4, 2)));
    MergeCoplanarFaces(pbox);

    Skeleton3D skel;
    skel.BuildForPolyMesh(pbox);

    // 8 original points + 2 center points for bisectors on each of 4
    // rectangular sides + 2 on square sides.
    EXPECT_EQ(18U, skel.GetVertices().size());

    // Get indices for original points and center points.
    std::vector<size_t> indices;
    for (size_t i = 0; i < pbox.vertices.size(); ++i)
        indices.push_back(FindIndex(skel, pbox.vertices[i]->point, i));
    indices.push_back(FindIndex(skel, Point3f(-1, -1,  0), -1));  // 6
    indices.push_back(FindIndex(skel, Point3f(-1,  1,  0), -1));  // 7
    indices.push_back(FindIndex(skel, Point3f( 1, -1,  0), -1));  // 8
    indices.push_back(FindIndex(skel, Point3f( 1,  1,  0), -1));  // 9
    indices.push_back(FindIndex(skel, Point3f( 0, -2,  0), -1));  // 10
    indices.push_back(FindIndex(skel, Point3f( 0,  2,  0), -1));  // 11
    indices.push_back(FindIndex(skel, Point3f( 0, -1, -1), -1));  // 12
    indices.push_back(FindIndex(skel, Point3f( 0,  1, -1), -1));  // 13
    indices.push_back(FindIndex(skel, Point3f( 0, -1,  1), -1));  // 14
    indices.push_back(FindIndex(skel, Point3f( 0,  1,  1), -1));  // 15

    // 4 bisectors on top and bottom faces; 5 on the others.
    EXPECT_EQ(28U, skel.GetEdges().size());

    EXPECT_TRUE(HasEdge(skel, indices[0], indices[8], 1, 3));
    EXPECT_TRUE(HasEdge(skel, indices[1], indices[8], 2, 0));
    EXPECT_TRUE(HasEdge(skel, indices[3], indices[9], 3, 1));
    EXPECT_TRUE(HasEdge(skel, indices[2], indices[9], 0, 2));

    EXPECT_TRUE(HasEdge(skel, indices[2], indices[13], 2, 7));
    EXPECT_TRUE(HasEdge(skel, indices[3], indices[13], 6, 3));
    EXPECT_TRUE(HasEdge(skel, indices[7], indices[13], 7, 2));
    EXPECT_TRUE(HasEdge(skel, indices[6], indices[13], 3, 6));

    EXPECT_TRUE(HasEdge(skel, indices[4], indices[10], 7, 10));
    EXPECT_TRUE(HasEdge(skel, indices[6], indices[11], 6, 9));
    EXPECT_TRUE(HasEdge(skel, indices[7], indices[11], 10, 7));
    EXPECT_TRUE(HasEdge(skel, indices[5], indices[10], 9, 6));

    EXPECT_TRUE(HasEdge(skel, indices[1], indices[12], 0, 10));
    EXPECT_TRUE(HasEdge(skel, indices[0], indices[12], 9, 1));
    EXPECT_TRUE(HasEdge(skel, indices[4], indices[12], 10, 0));
    EXPECT_TRUE(HasEdge(skel, indices[5], indices[12], 1, 9));

    EXPECT_TRUE(HasEdge(skel, indices[3], indices[17], 1, 6));
    EXPECT_TRUE(HasEdge(skel, indices[1], indices[16], 10, 2));
    EXPECT_TRUE(HasEdge(skel, indices[5], indices[16], 6, 1));
    EXPECT_TRUE(HasEdge(skel, indices[7], indices[17], 2, 10));

    EXPECT_TRUE(HasEdge(skel, indices[4], indices[14], 0, 7));
    EXPECT_TRUE(HasEdge(skel, indices[0], indices[14], 3, 9));
    EXPECT_TRUE(HasEdge(skel, indices[2], indices[15], 7, 0));
    EXPECT_TRUE(HasEdge(skel, indices[6], indices[15], 9, 3));

    // Internal edges.
    EXPECT_TRUE(HasEdge(skel, indices[8],  indices[9],  -1, -1));
    EXPECT_TRUE(HasEdge(skel, indices[10], indices[11], -1, -1));
    EXPECT_TRUE(HasEdge(skel, indices[17], indices[16], -1, -1));
    EXPECT_TRUE(HasEdge(skel, indices[14], indices[15], -1, -1));
}
