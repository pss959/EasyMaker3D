#include "Math/Linear.h"
#include "Math/Types.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class ModelMeshBinaryTest : public TestBase {};

TEST_F(ModelMeshBinaryTest, Empty) {

    // Empty string should fail.
    ModelMesh new_mesh;
    EXPECT_FALSE(new_mesh.FromBinaryString(""));
}

TEST_F(ModelMeshBinaryTest, Real) {
    // Create a ModelMesh with nonsense data.
    ModelMesh mesh;
    for (size_t i = 0; i < 12U; ++i) {
        mesh.points.push_back(Point3f(i, 2 * i, 3 * i));
        mesh.normals.push_back(GetAxis(i % 3));
        mesh.tex_coords.push_back(Point2f(4 * i, 5 * i));
    }
    for (size_t i = 0; i < 24U; ++i) {
        mesh.indices.push_back(i % 12U);
    }

    // Write it as a binary string.
    const auto str = mesh.ToBinaryString();

    // Read from the binary string and compare the results.
    ModelMesh new_mesh;
    EXPECT_TRUE(new_mesh.FromBinaryString(str));
    EXPECT_EQ(mesh.points,     new_mesh.points);
    EXPECT_EQ(mesh.normals,    new_mesh.normals);
    EXPECT_EQ(mesh.tex_coords, new_mesh.tex_coords);
    EXPECT_EQ(mesh.indices,    new_mesh.indices);
}
