#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshValidation.h"
#include "Math/MeshUtils.h"
#include "Math/SlicedMesh.h"  // XXXX
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

#include "Debug/Dump3dv.h" // XXXX

class SliceTest : public TestBase {};

TEST_F(SliceTest, BoxOneSlice) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const Vector3f dir(0, 1, 0);
    const SlicedMesh sm = NSliceMesh(box, dir, std::vector<float>{ .25f});
    EXPECT_EQ(dir,              sm.dir);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(16U,              sm.mesh.points.size());
    EXPECT_EQ(28U,              sm.mesh.GetTriangleCount());
    // XXXX More...

    Debug::Dump3dv d("/tmp/sliced.3dv", "XXXX From SliceTest");
    d.AddTriMesh(sm.mesh);
}

TEST_F(SliceTest, BoxTwoSlices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const Vector3f dir(0, 1, 0);
    const SlicedMesh sm = NSliceMesh(box, dir, std::vector<float>{.25f, .75f});
    EXPECT_EQ(dir,              sm.dir);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(24U,              sm.mesh.points.size());
    EXPECT_EQ(44U,              sm.mesh.GetTriangleCount());
    // XXXX More...

    Debug::Dump3dv d("/tmp/sliced2.3dv", "XXXX From SliceTest");
    d.SetLabelFontSize(60);
    d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));
    d.AddTriMesh(sm.mesh);
}
