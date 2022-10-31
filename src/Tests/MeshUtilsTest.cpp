#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "TestBase.h"
#include "Testing.h"

class MeshUtilsTest : public TestBase {
  protected:
    static constexpr float kMeshSize = 10;

    // Builds and returns a test mesh containing 4 points and 2 triangles
    // forming a square: -10 to +10 in X and Y, 0 in Z.
    static TriMesh BuildTestMesh() {
        TriMesh mesh;
        mesh.points.assign({ Point3f(-kMeshSize, -kMeshSize, 0),
                             Point3f(-kMeshSize,  kMeshSize, 0),
                             Point3f( kMeshSize,  kMeshSize, 0),
                             Point3f( kMeshSize, -kMeshSize, 0) });
        mesh.indices.assign({ 0, 1, 2,  0, 2, 3 });
        return mesh;
    }
};

TEST_F(MeshUtilsTest, TransformMesh) {
    const TriMesh m1 = BuildTestMesh();

    const Vector3f s(2, 3, 4);
    const Vector3f t(100, 200, 300);
    const Matrix4f m = GetTransformMatrix(s, Rotationf::Identity(), t);

    const TriMesh m2 = TransformMesh(m1, m);
    EXPECT_EQ(m1.points.size(),  m2.points.size());
    EXPECT_EQ(m1.indices.size(), m2.indices.size());
    for (size_t i = 0; i < m1.points.size(); ++i)
        EXPECT_PTS_CLOSE(Point3f(s) * m1.points[i] + t, m2.points[i]);
    for (size_t i = 0; i < m1.indices.size(); ++i)
        EXPECT_EQ(m1.indices[i], m2.indices[i]);
}

TEST_F(MeshUtilsTest, MirrorMesh) {
    // Make a Mesh that is not centered on the origin.
    const float trans = 2 * kMeshSize;
    const TriMesh m1 = TransformMesh(
        BuildTestMesh(),
        ion::math::TranslationMatrix(trans * Vector3f::AxisX()));
    const Bounds b1 = ComputeMeshBounds(m1);

    // Mirror about X=0.
    const TriMesh m2 = MirrorMesh(m1, Plane(0, Vector3f::AxisX()));
    const Bounds b2 = ComputeMeshBounds(m2);

    // Bounds should be the same size, but mirrored in X.
    EXPECT_EQ(b1.GetSize(), b2.GetSize());
    EXPECT_EQ(Point3f( trans, 0, 0), b1.GetCenter());
    EXPECT_EQ(Point3f(-trans, 0, 0), b2.GetCenter());
}

TEST_F(MeshUtilsTest, CenterMesh) {
    // Make a Mesh that is not centered on the origin.
    const float trans = 15.5;
    TriMesh m = TransformMesh(
        BuildTestMesh(),
        ion::math::TranslationMatrix(trans * Vector3f::AxisX()));
    const Bounds b1 = ComputeMeshBounds(m);

    // Center it.
    const TriMesh m2 = CenterMesh(m);
    const Bounds b2 = ComputeMeshBounds(m2);

    // Bounds should be the same size, but centered on the origin.
    EXPECT_EQ(b1.GetSize(), b2.GetSize());
    EXPECT_EQ(Point3f(trans, 0, 0), b1.GetCenter());
    EXPECT_EQ(Point3f(0, 0, 0),     b2.GetCenter());
}
