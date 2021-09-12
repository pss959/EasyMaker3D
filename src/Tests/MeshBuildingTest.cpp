#include "Math/CGALInterface.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Testing.h"

class MeshBuildingTest : public TestBase {};

TEST_F(MeshBuildingTest, Tetrahedron) {
    const TriMesh mesh = BuildTetrahedronMesh(10);
    EXPECT_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(10, 10, 10), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Box) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    EXPECT_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_EQ(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_EQ(Vector3f(10, 20, 30), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Cylinder) {
    const TriMesh mesh = BuildCylinderMesh(4, 8, 20, 20);
    EXPECT_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),      bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(16, 20, 16), bounds.GetSize());
}

TEST_F(MeshBuildingTest, Sphere) {
    const TriMesh mesh = BuildSphereMesh(4, 32, 32);
    EXPECT_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),   bounds.GetCenter());

    // Need extra large tolerance for this.
    EXPECT_PRED3(VectorsCloseT, Vector3f(8, 8, 8), bounds.GetSize(), .01f);
}

TEST_F(MeshBuildingTest, Torus) {
    const TriMesh mesh = BuildTorusMesh(4, 16, 20, 20);
    EXPECT_EQ(MeshValidityCode::kValid, IsMeshValid(mesh));
    Bounds bounds = ComputeMeshBounds(mesh);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 0),     bounds.GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(32, 8, 32), bounds.GetSize());
}
