#include <ion/math/vectorutils.h>

#include "Defaults.h"
#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshValidation.h"
#include "Math/MeshUtils.h"
#include "Math/Polygon.h"
#include "Math/TextUtils.h"
#include "TestBase.h"
#include "Testing.h"

class MeshBuildingTest : public TestBase {};

TEST_F(MeshBuildingTest, Tetrahedron) {
    const TriMesh mesh = BuildTetrahedronMesh(10);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(10, 10, 10), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Box) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(10, 20, 30), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Cylinder) {
    const TriMesh mesh = BuildCylinderMesh(4, 8, 20, 20);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
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

TEST_F(MeshBuildingTest, Sphere) {
    const TriMesh mesh = BuildSphereMesh(4, 32, 32);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   bounds.GetCenter());

    // Need extra large tolerance for this.
    EXPECT_PRED3(VectorsCloseT, Vector3f(8, 8, 8), bounds.GetSize(), .01f);
}

TEST_F(MeshBuildingTest, Torus) {
    const TriMesh mesh = BuildTorusMesh(4, 16, 20, 20);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(32, 8, 32), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Extruded) {
    // Just a triangle.
    Polygon poly(std::vector<Point2f>{ {-4, -4}, {4, -4}, {-4, 4} });
    const TriMesh mesh = BuildExtrudedMesh(poly, 10);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
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
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 5, 0),     bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(6, 10, 10), bounds.GetSize());
}

TEST_F(MeshBuildingTest, ExtrudedText) {
    std::vector<Polygon> polys =
        GetTextOutlines(Defaults::kFontName, "A", 0, 1);
    EXPECT_EQ(1U, polys.size());
    const TriMesh mesh = BuildExtrudedMesh(polys[0], 10);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
}
