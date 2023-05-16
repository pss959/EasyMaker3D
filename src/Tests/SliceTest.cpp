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
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(16U,              sm.mesh.points.size());
    EXPECT_EQ(28U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box2Slices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kY,
                                    std::vector<float>{.25f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(24U,              sm.mesh.points.size());
    EXPECT_EQ(44U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box3Slices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kY,
                                    std::vector<float>{.25f, .6f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(32U,              sm.mesh.points.size());
    EXPECT_EQ(60U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box3SlicesX) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kX,
                                    std::vector<float>{.25f, .6f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kX,       sm.axis);
    EXPECT_EQ(Range1f(-5, 5), sm.range);
    EXPECT_EQ(32U,            sm.mesh.points.size());
    EXPECT_EQ(60U,            sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Cylinder) {
    const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 20);

    const SlicedMesh sm = SliceMesh(cyl, Axis::kY,
                                    std::vector<float>{.25f, .6f, .75f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(162U,             sm.mesh.points.size());
    EXPECT_EQ(320U,             sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, CylinderX) {
    // EnableKLog("X"); // XXXX

    // XXXX const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 20);
    const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 8);

#if XXXX
    SlicedMesh sm = SliceMesh(cyl, Axis::kX,
                              std::vector<float>{.2f, .6f, .75f});
#endif
    SlicedMesh sm = SliceMesh(cyl, Axis::kX,
                              std::vector<float>{.2f});
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateAndRepairTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kX,       sm.axis);
    EXPECT_EQ(Range1f(-8, 8), sm.range);
    EXPECT_EQ(162U,           sm.mesh.points.size());
    EXPECT_EQ(320U,           sm.mesh.GetTriangleCount());

#if 1 // XXXX
    const auto border_indices = GetBorderEdges(sm.mesh);
    const auto highlight_edge = [&border_indices](GIndex v0, GIndex v1){
        for (size_t i = 0; i < border_indices.size(); i += 2) {
            if (v0 == border_indices[i] && v1 == border_indices[i + 1])
                return true;
        }
        return false;
    };
    const auto highlight_vert = [&border_indices](GIndex v){
        return std::find(border_indices.begin(), border_indices.end(), v) !=
            border_indices.end();
    };

    Debug::Dump3dv d("/tmp/sliced.3dv", "XXXX From SliceTest");
    //d.SetLabelFlags(Debug::Dump3dv::LabelFlags());
    d.SetLabelFontSize(40);
    d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));

    d.AddTriMesh(sm.mesh, nullptr, highlight_edge, highlight_vert);
#endif
}
