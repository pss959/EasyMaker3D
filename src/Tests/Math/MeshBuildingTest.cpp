#include <algorithm>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshValidation.h"
#include "Math/MeshUtils.h"
#include "Math/Polygon.h"
#include "Math/Profile.h"
#include "Math/TextUtils.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class MeshBuildingTest : public TestBase {};

TEST_F(MeshBuildingTest, Tetrahedron) {
    const TriMesh mesh = BuildTetrahedronMesh(10);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(10, 10, 10), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Box) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(10, 20, 30), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Cylinder) {
    const TriMesh mesh   = BuildCylinderMesh(4, 8, 20, 20);
    const Bounds  bounds = ComputeMeshBounds(mesh);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),      bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(16, 20, 16), bounds.GetSize());

    // Validate orientation of all triangles.
    for (size_t i = 0; i < mesh.GetTriangleCount(); ++i) {
        const Point3f &p0 = mesh.points[mesh.indices[3 * i + 0]];
        const Point3f &p1 = mesh.points[mesh.indices[3 * i + 1]];
        const Point3f &p2 = mesh.points[mesh.indices[3 * i + 2]];
        const Vector3f norm = ComputeNormal(p0, p1, p2);
        // Make sure the vector from the center to a point is in the same rough
        // direction as the triangle normal.
        const Vector3f vec = ion::math::Normalized(p0 - Point3f::Zero());
        EXPECT_GT(ion::math::Dot(norm, vec), 0);
    }
}

TEST_F(MeshBuildingTest, CylinderCone) {
    // A radius of 0 creates a cone.
    const TriMesh mesh1   = BuildCylinderMesh(0, 8, 20, 20);
    const Bounds  bounds1 = ComputeMeshBounds(mesh1);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh1));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),      bounds1.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(16, 20, 16), bounds1.GetSize());

    // Same, but upside down.
    const TriMesh mesh2   = BuildCylinderMesh(8, 0, 20, 20);
    const Bounds  bounds2 = ComputeMeshBounds(mesh2);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh2));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),      bounds2.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(16, 20, 16), bounds2.GetSize());
}

TEST_F(MeshBuildingTest, RevSurf) {
    // Default profile, full sweep.
    const Profile prof =
        Profile::CreateFixedProfile(Point2f(0, 1), Point2f(0, 0), 3,
                                    Profile::PointVec{ Point2f(.5f, .5f) });
    const TriMesh mesh1 = BuildRevSurfMesh(prof, Anglef::FromDegrees(360), 20);
    const Bounds  bounds1 = ComputeMeshBounds(mesh1);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh1));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   bounds1.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 1), bounds1.GetSize());

    // Default profile, half sweep.
    const TriMesh mesh2 = BuildRevSurfMesh(prof, Anglef::FromDegrees(180), 20);
    const Bounds  bounds2 = ComputeMeshBounds(mesh2);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh2));
    EXPECT_PTS_CLOSE(Point3f(0, 0, .25f),  bounds2.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(1, 1, .5f), bounds2.GetSize());

    // Add point to profile, still half sweep.
    Profile prof2 = prof;
    prof2.AppendPoint(Point2f(.25f, .5f));
    const TriMesh mesh3 = BuildRevSurfMesh(prof2, Anglef::FromDegrees(180), 20);
    const Bounds  bounds3 = ComputeMeshBounds(mesh3);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh3));
    EXPECT_PTS_CLOSE(Point3f(0, 0, .25f),  bounds3.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(1, 1, .5f), bounds3.GetSize());
}

TEST_F(MeshBuildingTest, Sphere) {
    const TriMesh mesh = BuildSphereMesh(4, 32, 32);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   bounds.GetCenter());

    // Need extra large tolerance for this.
    EXPECT_PRED3(VectorsCloseT, Vector3f(8, 8, 8), bounds.GetSize(), .01f);
}

TEST_F(MeshBuildingTest, Torus) {
    const TriMesh mesh = BuildTorusMesh(4, 16, 20, 20);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(32, 8, 32), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Tube) {
    // One segment, diameter 4, square cross section, no taper.
    std::vector<Point3f>pts{ Point3f(-10, 0, 0), Point3f(10, 0, 0) };
    TriMesh mesh = BuildTubeMesh(pts, 4, 1, 4);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),    bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(20, 4, 4), bounds.GetSize());

    // Same, with taper to .2 at the right end.
    mesh = BuildTubeMesh(pts, 4, .2f, 4);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),    bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(20, 4, 4), bounds.GetSize());
    EXPECT_TRUE(MeshHasPoint(mesh, Point3f(10, .4f, 0)));

    // 2 segments, no taper. The mesh is not valid because the segments do not
    // join.
    pts.insert(pts.begin() + 1, Point3f(2, 0, 0));
    mesh = BuildTubeMesh(pts, 10, 1, 4);
    EXPECT_NE(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),      bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(20, 10, 10), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Polygon) {
    // A triangle.
    const std::vector<Point2f> points{
        Point2f(0, 0),
        Point2f(1, 0),
        Point2f(0, 1),
    };
    Polygon poly(points);

    const TriMesh mesh = BuildPolygonMesh(poly);
    EXPECT_ENUM_EQ(MeshValidityCode::kNotClosed, ValidateTriMesh(mesh));
    EXPECT_EQ(3U, mesh.points.size());
    EXPECT_EQ(3U, mesh.indices.size());
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(.5f, .5f, 0), bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(1, 1, 0),   bounds.GetSize());
}

TEST_F(MeshBuildingTest, PolygonWithHole) {
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
    Polygon poly(points, border_counts);

    const TriMesh mesh = BuildPolygonMesh(poly);
    EXPECT_ENUM_EQ(MeshValidityCode::kNotClosed, ValidateTriMesh(mesh));
    EXPECT_EQ(8U,  mesh.points.size());
    EXPECT_EQ(24U, mesh.indices.size());
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 00, 0),   bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(6, 10, 0), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Extruded) {
    // Just a triangle.
    Polygon poly(std::vector<Point2f>{ {-4, -4}, {4, -4}, {-4, 4} });
    const TriMesh mesh = BuildExtrudedMesh(poly, 10);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 5, 0),    bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(8, 10, 8), bounds.GetSize());
}

TEST_F(MeshBuildingTest, ExtrudedWithHole) {
    Polygon poly(
        std::vector<Point2f>{
            {-3, -5}, {3, -5}, {3, 5}, {-3, 5},  // Outer border.
            {-1, -3}, {-1, 3}, {1, 3}, {1, -3},  // Hole (clockwise}.
        },
        std::vector<size_t> { 4, 4 });  // Border counts.
    const TriMesh mesh = BuildExtrudedMesh(poly, 10);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 5, 0),     bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(6, 10, 10), bounds.GetSize());
}

TEST_F(MeshBuildingTest, ExtrudedText) {
    std::vector<Polygon> polys = GetTextOutlines(TK::k3DFont, "A", 0, 1);
    EXPECT_EQ(1U, polys.size());
    const TriMesh mesh = BuildExtrudedMesh(polys[0], 10);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(mesh));
}
