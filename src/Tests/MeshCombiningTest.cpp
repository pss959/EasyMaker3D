#include "Math/MeshBuilding.h"
#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"
#include "TestBase.h"
#include "Testing.h"

class MeshCombiningTest : public TestBase {
};

TEST_F(MeshCombiningTest, ClipMesh) {
    // Build a 10x10x10 box centered on the origin and clip off the top half.
    TriMesh box = BuildBoxMesh(Vector3f(10, 10, 10));
    TriMesh clipped = ClipMesh(box, Plane(0, Vector3f::AxisY()));

    Bounds bounds = ComputeMeshBounds(clipped);
    EXPECT_EQ(Vector3f(10, 5, 10), bounds.GetSize());
    EXPECT_EQ(Point3f(0, -2.5f, 0), bounds.GetCenter());
}
