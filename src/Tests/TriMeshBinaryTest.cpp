#include "Math/MeshBuilding.h"
#include "Math/Types.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class TriMeshBinaryTest : public TestBase {};

TEST_F(TriMeshBinaryTest, Empty) {
    // Empty string should fail.
    TriMesh new_mesh;
    EXPECT_FALSE(new_mesh.FromBinaryString(""));
}

TEST_F(TriMeshBinaryTest, Real) {
    // Create a TriMesh and write it as a binary string.
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const auto str = mesh.ToBinaryString();

    // Read from the binary string and compare the results.
    TriMesh new_mesh;
    EXPECT_TRUE(new_mesh.FromBinaryString(str));
    EXPECT_EQ(mesh.points,  new_mesh.points);
    EXPECT_EQ(mesh.indices, new_mesh.indices);
}
