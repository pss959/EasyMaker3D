#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/PolyMesh.h"
#include "Testing.h"

class PolyMeshTest : public TestBase {};

TEST_F(PolyMeshTest, Box) {
    TriMesh m = BuildBoxMesh(Vector3f(10, 10, 10));
    PolyMesh poly_mesh(m);

    TriMesh rm = poly_mesh.ToTriMesh();

    EXPECT_EQ(8U,  rm.points.size());
    EXPECT_EQ(12U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Mesh from PolyMesh");
}
