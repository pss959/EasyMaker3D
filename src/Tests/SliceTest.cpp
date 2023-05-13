#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshValidation.h"
#include "Math/MeshUtils.h"
#include "Math/SlicedMesh.h"  // XXXX
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

#include "Debug/Dump3dv.h" // XXXX

class SliceTest : public TestBase {};

TEST_F(SliceTest, Box1Slice) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kY, std::vector<float>{ .25f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Vector3f::AxisY(), sm.dir);
    EXPECT_EQ(Range1f(-10, 10),  sm.range);
    EXPECT_EQ(16U,               sm.mesh.points.size());
    EXPECT_EQ(28U,               sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box2Slices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kY,
                                    std::vector<float>{.25f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Vector3f::AxisY(), sm.dir);
    EXPECT_EQ(Range1f(-10, 10),  sm.range);
    EXPECT_EQ(24U,               sm.mesh.points.size());
    EXPECT_EQ(44U,               sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box3Slices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kY,
                                    std::vector<float>{.25f, .6f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Vector3f::AxisY(), sm.dir);
    EXPECT_EQ(Range1f(-10, 10),  sm.range);
    EXPECT_EQ(32U,               sm.mesh.points.size());
    EXPECT_EQ(60U,               sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box3SlicesX) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kX,
                                    std::vector<float>{.25f, .6f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Vector3f::AxisX(), sm.dir);
    EXPECT_EQ(Range1f(-5, 5),    sm.range);
    EXPECT_EQ(32U,               sm.mesh.points.size());
    EXPECT_EQ(60U,               sm.mesh.GetTriangleCount());

#if 0 // XXXX
    Debug::Dump3dv d("/tmp/sliced.3dv", "XXXX From SliceTest");
    d.SetLabelFontSize(60);
    d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));
    d.AddTriMesh(sm.mesh);
#endif
}

TEST_F(SliceTest, Cylinder) {
    const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 20);

    const Vector3f dir(0, 1, 0);
    const SlicedMesh sm = SliceMesh(cyl, Axis::kY,
                                    std::vector<float>{.25f, .6f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Vector3f::AxisY(), sm.dir);
    EXPECT_EQ(Range1f(-10, 10),  sm.range);
    EXPECT_EQ(162U,              sm.mesh.points.size());
    EXPECT_EQ(320U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, CylinderX) {
    EnableKLog("|"); // XXXX

    const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 20);

    const Vector3f dir(0, 1, 0);
    const SlicedMesh sm = SliceMesh(cyl, Axis::kX,
                                    std::vector<float>{.25f, .6f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Vector3f::AxisX(), sm.dir);
    EXPECT_EQ(Range1f(-10, 10),  sm.range);
    EXPECT_EQ(162U,              sm.mesh.points.size());
    EXPECT_EQ(320U,              sm.mesh.GetTriangleCount());

#if 1 // XXXX
    Debug::Dump3dv d("/tmp/sliced.3dv", "XXXX From SliceTest");
    // d.SetLabelFlags(Debug::Dump3dv::LabelFlags());
    d.SetLabelFontSize(40);
    d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));
    d.AddTriMesh(sm.mesh);
#endif
}
