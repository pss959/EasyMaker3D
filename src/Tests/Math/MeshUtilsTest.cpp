#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"
#include "Math/Profile.h"
#include "Math/SlicedMesh.h"
#include "Math/Spin.h"
#include "Math/Taper.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

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

// ----------------------------------------------------------------------------
// Mesh transformations.
// ----------------------------------------------------------------------------

TEST_F(MeshUtilsTest, ScaleMesh) {
    const TriMesh m1 = BuildTestMesh();

    const Vector3f scale(2, 3, 4);
    const TriMesh m2 = ScaleMesh(m1, scale);
    EXPECT_EQ(m1.points.size(),  m2.points.size());
    for (size_t i = 0; i < m1.points.size(); ++i)
        EXPECT_PTS_CLOSE(ScalePoint(m1.points[i], scale), m2.points[i]);
    EXPECT_EQ(m1.indices, m2.indices);
}

TEST_F(MeshUtilsTest, RotateMesh) {
    const TriMesh m1 = BuildTestMesh();

    const Rotationf rot = BuildRotation(1, 2, -3, 40);
    const TriMesh m2 = RotateMesh(m1, rot);
    EXPECT_EQ(m1.points.size(),  m2.points.size());
    for (size_t i = 0; i < m1.points.size(); ++i)
        EXPECT_PTS_CLOSE(rot * m1.points[i], m2.points[i]);
    EXPECT_EQ(m1.indices, m2.indices);
}

TEST_F(MeshUtilsTest, TransformMesh) {
    const TriMesh m1 = BuildTestMesh();

    const Vector3f s(2, 3, 4);
    const Vector3f t(100, 200, 300);
    const Matrix4f m = GetTransformMatrix(s, Rotationf::Identity(), t);

    const TriMesh m2 = TransformMesh(m1, m);
    EXPECT_EQ(m1.points.size(),  m2.points.size());
    for (size_t i = 0; i < m1.points.size(); ++i)
        EXPECT_PTS_CLOSE(Point3f(s) * m1.points[i] + t, m2.points[i]);
    EXPECT_EQ(m1.indices, m2.indices);
}

TEST_F(MeshUtilsTest, BendMesh) {
    // Get a SlicedMesh consisting of a 2x2x10 box sliced into 4 slices along
    // the Z axis.
    const auto sm = SliceMesh(BuildBoxMesh(Vector3f(2, 2, 10)), Dim::kZ, 4);

    // Bend it 0 degrees. Should have no effect.
    Spin s;
    s.angle = Anglef::FromDegrees(0);
    const TriMesh m0 = BendMesh(sm, s);
    EXPECT_EQ(sm.mesh.points,  m0.points);
    EXPECT_EQ(sm.mesh.indices, m0.indices);

    // Bend it 90 degrees.
    s.angle = Anglef::FromDegrees(90);
    const TriMesh m1 = BendMesh(sm, s);
    EXPECT_EQ(32U,  m1.points.size());
    EXPECT_EQ(180U, m1.indices.size());

    // Test the bounds.
    const Bounds b1 = ComputeMeshBounds(m1);
    EXPECT_VECS_CLOSE(Vector3f(3.57172f, 2, 10.4174f), b1.GetSize());
    EXPECT_PTS_CLOSE(Point3f(.78586f, 0, 0),           b1.GetCenter());

    // Bend it to test the inside-out case.
    s.center.Set(10, 0, 0);
    s.angle = Anglef::FromDegrees(165);
    const TriMesh m2 = BendMesh(sm, s);
    EXPECT_EQ(32U,  m2.points.size());
    EXPECT_EQ(180U, m2.indices.size());
    const Bounds b2 = ComputeMeshBounds(m2);
    EXPECT_VECS_CLOSE(Vector3f(6.80604f, 2, 14.9263f), b2.GetSize());
    EXPECT_PTS_CLOSE(Point3f(2.40302f, 0, 0),          b2.GetCenter());

    // Test the 360-degree case.
    s.center.Set(0, 0, 0);
    s.angle = Anglef::FromDegrees(360);
    const TriMesh m3 = BendMesh(sm, s);
    EXPECT_EQ(28U,  m3.points.size());
    EXPECT_EQ(168U, m3.indices.size());
    const Bounds b3 = ComputeMeshBounds(m3);
    EXPECT_VECS_CLOSE(Vector3f(5.1831f, 2, 5.1831f), b3.GetSize());
    EXPECT_PTS_CLOSE(Point3f(1.59155f, 0, 0),        b3.GetCenter());
}

TEST_F(MeshUtilsTest, BendCylMesh) {
    // Tests bending a low-complexity 2x10x2 cylinder 360 degrees around the X
    // axis; this requires the center vertices on the caps to be removed.
    const auto sm = SliceMesh(BuildCylinderMesh(1, 1, 10, 4), Dim::kY, 6);

    Spin s;
    s.center.Set(0, 0, 0);
    s.axis.Set(1, 0, 0);
    s.angle = Anglef::FromDegrees(360);
    const TriMesh m = BendMesh(sm, s);
    EXPECT_EQ(44U,  m.points.size());
    EXPECT_EQ(264U, m.indices.size());
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

TEST_F(MeshUtilsTest, TaperMesh) {
    // Get a SlicedMesh consisting of a 2x10x2 box sliced into 4 slices along
    // the Y axis.
    const auto sm = SliceMesh(BuildBoxMesh(Vector3f(2, 10, 2)), Dim::kY, 4);

    // Taper it to 0 at the top and bottom and 1 in the middle.
    const Profile::PointVec pts{
        Point2f(0, 1),
        Point2f(1, .5f),
        Point2f(0, 0)
    };
    Taper t;
    t.profile = Profile(Profile::Type::kOpen, 2, pts);
    EXPECT_TRUE(Taper::IsValidProfile(t.profile));

    const TriMesh m = TaperMesh(sm, t);
    EXPECT_EQ(32U,  m.points.size());
    EXPECT_EQ(180U, m.indices.size());

    // Test for various expected points.
    EXPECT_TRUE(MeshHasPoint(m, Point3f(0,  5, 0)));  // Top point.
    EXPECT_TRUE(MeshHasPoint(m, Point3f(0, -5, 0)));  // Bottom point.
    EXPECT_TRUE(MeshHasPoint(m, Point3f(-1, 0, 0)));
    EXPECT_TRUE(MeshHasPoint(m, Point3f(-1, 0, -1)));
    EXPECT_TRUE(MeshHasPoint(m, Point3f(-1, 0,  1)));
    EXPECT_TRUE(MeshHasPoint(m, Point3f( 1, 0, -1)));
    EXPECT_TRUE(MeshHasPoint(m, Point3f( 1, 0,  1)));
    EXPECT_TRUE(MeshHasPoint(m, Point3f(.25f, 2.5f, .5f)));

    // Test the bounds.
    const Bounds b = ComputeMeshBounds(m);
    EXPECT_VECS_CLOSE(Vector3f(2, 10, 2), b.GetSize());
    EXPECT_PTS_CLOSE(Point3f::Zero(),     b.GetCenter());
}

TEST_F(MeshUtilsTest, TwistMesh) {
    // Get a SlicedMesh consisting of a 2x10x2 box sliced into 4 slices along
    // the Y axis.
    const auto sm = SliceMesh(BuildBoxMesh(Vector3f(2, 10, 2)), Dim::kY, 4);

    // Twist it 90 degrees.
    Spin s;
    s.angle = Anglef::FromDegrees(90);
    const TriMesh m = TwistMesh(sm, s);

    EXPECT_EQ(32U,  m.points.size());
    EXPECT_EQ(180U, m.indices.size());

    // Test the bounds.
    const Bounds b = ComputeMeshBounds(m);
    EXPECT_VECS_CLOSE(Vector3f(2.82843f, 10, 2.82843f), b.GetSize());
    EXPECT_PTS_CLOSE(Point3f::Zero(),                   b.GetCenter());
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

TEST_F(MeshUtilsTest, UnshareMeshVertices) {
    TriMesh m = BuildBoxMesh(Vector3f(8, 8, 8));
    UnshareMeshVertices(m);
    EXPECT_EQ(36U, m.points.size());
    EXPECT_EQ(36U, m.indices.size());
}

// ----------------------------------------------------------------------------
// Mesh to Ion Shapes and vice-versa.
// ----------------------------------------------------------------------------

TEST_F(MeshUtilsTest, ToFromIonShape) {
    // Create a box.
    const TriMesh m0 = BuildBoxMesh(Vector3f(8, 8, 8));

    // Convert to an Ion shape.
    const auto shape = TriMeshToIonShape(m0, true, true, true);

    // Convert back to a TriMesh.
    const TriMesh m1 = IonShapeToTriMesh(*shape);

    EXPECT_EQ(m0.points,  m1.points);
    EXPECT_EQ(m0.indices, m1.indices);
}
