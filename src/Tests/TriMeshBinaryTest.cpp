#include "Math/MeshBuilding.h"
#include "Math/Types.h"
#include "TestBase.h"
#include "Testing.h"

class TriMeshReadWriteTest : public TestBase {};

TEST_F(TriMeshReadWriteTest, ReadWrite) {
    // Create a TriMesh and write it as a binary string.
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const auto str = mesh.ToBinaryString();

    TriMesh new_mesh;

    // Empty string should fail.
    EXPECT_FALSE(new_mesh.FromBinaryString(""));

    // Read from the binary string and compare the results.
    EXPECT_TRUE(new_mesh.FromBinaryString(str));
    EXPECT_EQ(mesh.points,  new_mesh.points);
    EXPECT_EQ(mesh.indices, new_mesh.indices);
}
