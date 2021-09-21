#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/Solid.h"
#include "Testing.h"

class SolidTest : public TestBase {};

TEST_F(SolidTest, Box) {
    TriMesh m = BuildBoxMesh(Vector3f(10, 10, 10));
    Solid solid(m);

    TriMesh rm = solid.ToTriMesh();

    EXPECT_EQ(8U,  rm.points.size());
    EXPECT_EQ(12U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Mesh from Solid");
}
