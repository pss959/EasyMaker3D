#include "Math/MeshBuilding.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

// These are needed for the mesh dump functions.
#include "Debug/Dump3dv.h"
#include "Math/Linear.h"
#include "Util/General.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// SliceTest fixture.
// ----------------------------------------------------------------------------

class SliceTest : public TestBase {
  protected:
    // Enables slicing logging.
    void EnableSliceLogging() { EnableKLog("X"); }

    // This can be used in tests to dump a TriMesh in 3dv format to /tmp.
    void DumpTriMesh(const std::string &name, const TriMesh &mesh,
                     bool add_labels = true);

    // This can be used in tests to dump a TriMesh and slicing planes for the
    // given fractions in 3dv format to /tmp.
    void DumpTriMesh(const std::string &name, const TriMesh &mesh,
                     int dim, const std::vector<float> &fractions,
                     const Range1f &range, bool add_labels = true);
};

void SliceTest::DumpTriMesh(const std::string &name, const TriMesh &mesh,
                            bool add_labels) {
    Debug::Dump3dv d("/tmp/" + name + ".3dv", "XXXX From SliceTest");
    if (! add_labels)
        d.SetLabelFlags(Debug::Dump3dv::LabelFlags());
    d.SetLabelFontSize(40);
    d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));
    d.AddTriMesh(mesh);
};

void SliceTest::DumpTriMesh(const std::string &name, const TriMesh &mesh,
                            int dim, const std::vector<float> &fractions,
                            const Range1f &range, bool add_labels) {
    Debug::Dump3dv d("/tmp/" + name + ".3dv", "XXXX From SliceTest");
    if (! add_labels)
        d.SetLabelFlags(Debug::Dump3dv::LabelFlags());
    d.SetLabelFontSize(40);
    d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));
    d.AddTriMesh(mesh);

    // Add rectangles showing slicing planes.
    const auto sz = .8f * ComputeMeshBounds(mesh).GetSize();
    const int dim1 = (dim + 1) % 3;
    const int dim2 = (dim + 2) % 3;
    ASSERT(dim  >= 0 && dim  <= 2);
    ASSERT(dim1 >= 0 && dim1 <= 2);
    ASSERT(dim2 >= 0 && dim2 <= 2);
    for (size_t i = 0; i < fractions.size(); ++i) {
        const float frac = fractions[i];
        const std::string fid = "SPF" + Util::ToString(i);
        const float val = Lerp(frac, range.GetMinPoint(), range.GetMaxPoint());
        for (int j = 0; j < 4; ++j) {
            Point3f p;
            p[dim]  = val;
            p[dim1] = (j == 0 || j == 1) ? -sz[dim1] : sz[dim1];
            p[dim2] = (j == 0 || j == 3) ? -sz[dim2] : sz[dim2];
            d.AddVertex(fid + "_" + Util::ToString(j), p);
        }
        d.AddFace(fid, std::vector<std::string>{
                fid + "_0", fid + "_1", fid + "_2", fid + "_3"});
    }
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(SliceTest, Box0Slices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const SlicedMesh sm = SliceMesh(box, Axis::kY, std::vector<float>());
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(8U,               sm.mesh.points.size());
    EXPECT_EQ(12U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box1Slice) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const std::vector<float> fractions{.25f};
    const SlicedMesh sm = SliceMesh(box, Axis::kY, fractions);
    DumpTriMesh("unsliced", box, 1, fractions, sm.range);
    DumpTriMesh("sliced",   sm.mesh);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(16U,              sm.mesh.points.size());
    EXPECT_EQ(28U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box2Slices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const std::vector<float> fractions{.25f, .75f};
    const SlicedMesh sm = SliceMesh(box, Axis::kY, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(24U,              sm.mesh.points.size());
    EXPECT_EQ(44U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box3Slices) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const std::vector<float> fractions{.25f, .6f, .75f};
    const SlicedMesh sm = SliceMesh(box, Axis::kY, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(32U,              sm.mesh.points.size());
    EXPECT_EQ(60U,              sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Box3SlicesX) {
    const TriMesh box = BuildBoxMesh(Vector3f(10, 20, 30));

    const std::vector<float> fractions{.25f, .6f, .75f};
    const SlicedMesh sm = SliceMesh(box, Axis::kX, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kX,       sm.axis);
    EXPECT_EQ(Range1f(-5, 5), sm.range);
    EXPECT_EQ(32U,            sm.mesh.points.size());
    EXPECT_EQ(60U,            sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, Cylinder) {
    const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 20);

    const std::vector<float> fractions{.25f, .6f, .75f};
    const SlicedMesh sm = SliceMesh(cyl, Axis::kY, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kY,         sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(162U,             sm.mesh.points.size());
    EXPECT_EQ(320U,             sm.mesh.GetTriangleCount());
}

TEST_F(SliceTest, CylinderX) {
    // XXXX const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 20);
    const TriMesh cyl = BuildCylinderMesh(4, 8, 20, 8);

    const std::vector<float> fractions{.2f, .6f, .75f};
    // const std::vector<float> fractions{.2f}; // XXXX TEMP

    SlicedMesh sm = SliceMesh(cyl, Axis::kX, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateAndRepairTriMesh(sm.mesh));
    EXPECT_EQ(Axis::kX,       sm.axis);
    EXPECT_EQ(Range1f(-8, 8), sm.range);
    EXPECT_EQ(162U,           sm.mesh.points.size());
    EXPECT_EQ(320U,           sm.mesh.GetTriangleCount());
}
