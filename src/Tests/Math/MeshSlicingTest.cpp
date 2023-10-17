#include "Math/MeshBuilding.h"
#include "Math/MeshSlicing.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

// These are needed for the mesh dump functions.
#include <ion/math/vectorutils.h>
#include "Debug/Dump3dv.h"
#include "Math/Linear.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// MeshSlicingTest fixture.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class MeshSlicingTest : public TestBase {
  protected:
    /// Data passed to DumpTriMesh.
    ///
    /// For example, to dump the original (unsliced) mesh:
    ///   DumpData data;
    ///   data.fractions = fractions;
    ///   data.dim = Util::EnumInt(sm.axis);
    ///   data.range = sm.range;
    ///   DumpTriMesh("unsliced", mesh, data);
    ///
    /// Or to dump the result (sliced) mesh with borders highlighted:
    ///   DumpData data;
    ///   data.add_face_labels = false;
    ///   data.add_edge_labels = false;
    ///   data.highlight_borders = true;
    ///   DumpTriMesh("sliced", sm.mesh, data);
    struct DumpData {
        bool add_face_labels   = true;
        bool add_edge_labels   = true;
        bool add_vertex_labels = true;

        // Set these to add slicing plane rectangles.
        std::vector<float> fractions;
        Range1f range;
        int     dim = 1;

        // Set this to true to highlight border edges and vertices.
        bool highlight_borders = false;
    };

    //// Enables slicing logging.
    void EnableSliceLogging() { KLogger::SetKeyString("X"); }

    /// This can be used in tests to dump a TriMesh in 3dv format to /tmp.
    void DumpTriMesh(const Str &file_name, const TriMesh &mesh,
                     const DumpData &data);
};

void MeshSlicingTest::DumpTriMesh(const Str &file_name,
                                  const TriMesh &mesh, const DumpData &data) {
    Debug::Dump3dv d("/tmp/" + file_name + ".3dv", "From MeshSlicingTest");
    const auto sz = .8f * ComputeMeshBounds(mesh).GetSize();

    Debug::Dump3dv::LabelFlags flags;
    if (data.add_face_labels)
        flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
    if (data.add_edge_labels)
        flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
    if (data.add_vertex_labels)
        flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
    d.SetLabelFlags(flags);
    d.SetLabelFontSize(1.2f * ion::math::Length(sz));
    d.SetCoincidentLabelOffset(.25f * Vector3f(1, 1, 1));

    // Set up highlighting if requested.
    if (data.highlight_borders) {
        const auto border_indices = GetBorderEdges(mesh);
        const auto highlight_edge = [&border_indices](GIndex v0, GIndex v1){
            for (size_t i = 0; i < border_indices.size(); i += 2) {
                if (v0 == border_indices[i] && v1 == border_indices[i + 1])
                    return true;
            }
            return false;
        };
        const auto highlight_vert = [&border_indices](GIndex v){
            return Util::Contains(border_indices, v);
        };
        d.AddTriMesh(mesh, nullptr, highlight_edge, highlight_vert);
    }
    else {
        d.AddTriMesh(mesh);
    }

    // Add rectangles showing slicing planes.
    if (! data.fractions.empty()) {
        const int dim0 = data.dim;
        const int dim1 = (dim0 + 1) % 3;
        const int dim2 = (dim0 + 2) % 3;
        ASSERT_TRUE(dim0 >= 0 && dim0 <= 2);
        ASSERT_TRUE(dim1 >= 0 && dim1 <= 2);
        ASSERT_TRUE(dim2 >= 0 && dim2 <= 2);
        for (size_t i = 0; i < data.fractions.size(); ++i) {
            const float frac = data.fractions[i];
            const Str fid = "SPF" + Util::ToString(i);
            const float val =
                Lerp(frac, data.range.GetMinPoint(), data.range.GetMaxPoint());
            for (int j = 0; j < 4; ++j) {
                Point3f p;
                p[dim0] = val;
                p[dim1] = (j == 0 || j == 1) ? -sz[dim1] : sz[dim1];
                p[dim2] = (j == 0 || j == 3) ? -sz[dim2] : sz[dim2];
                d.AddVertex(fid + "_" + Util::ToString(j), p);
            }
            d.AddFace(fid,
                      StrVec{fid + "_0", fid + "_1", fid + "_2", fid + "_3"});
        }
    }
};

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(MeshSlicingTest, Box0Slices) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const SlicedMesh sm = SliceMesh(mesh, Dim::kY, std::vector<float>());
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kY,          sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(8U,               sm.mesh.points.size());
    EXPECT_EQ(12U,              sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, Box1Slice) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const std::vector<float> fractions{.25f};
    const SlicedMesh sm = SliceMesh(mesh, Dim::kY, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kY,          sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(16U,              sm.mesh.points.size());
    EXPECT_EQ(28U,              sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, Box1SliceEven) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const SlicedMesh sm = SliceMesh(mesh, Dim::kY, 2);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kY,          sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(16U,              sm.mesh.points.size());
    EXPECT_EQ(28U,              sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, Box2Slices) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const std::vector<float> fractions{.25f, .75f};
    const SlicedMesh sm = SliceMesh(mesh, Dim::kY, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kY,          sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(24U,              sm.mesh.points.size());
    EXPECT_EQ(44U,              sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, Box3Slices) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const std::vector<float> fractions{.25f, .6f, .75f};
    const SlicedMesh sm = SliceMesh(mesh, Dim::kY, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kY,          sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(32U,              sm.mesh.points.size());
    EXPECT_EQ(60U,              sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, Box3SlicesX) {
    const TriMesh mesh = BuildBoxMesh(Vector3f(10, 20, 30));
    const std::vector<float> fractions{.25f, .6f, .75f};
    const SlicedMesh sm = SliceMesh(mesh, Dim::kX, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kX,        sm.axis);
    EXPECT_EQ(Range1f(-5, 5), sm.range);
    EXPECT_EQ(32U,            sm.mesh.points.size());
    EXPECT_EQ(60U,            sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, Cylinder) {
    const TriMesh mesh = BuildCylinderMesh(4, 8, 20, 20);
    const std::vector<float> fractions{.25f, .6f, .75f};
    const SlicedMesh sm = SliceMesh(mesh, Dim::kY, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kY,          sm.axis);
    EXPECT_EQ(Range1f(-10, 10), sm.range);
    EXPECT_EQ(162U,             sm.mesh.points.size());
    EXPECT_EQ(320U,             sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, CylinderX1) {
    const TriMesh mesh = BuildCylinderMesh(4, 8, 20, 8);
    const std::vector<float> fractions{.75f};
    SlicedMesh sm = SliceMesh(mesh, Dim::kX, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateAndRepairTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kX,        sm.axis);
    EXPECT_EQ(Range1f(-8, 8), sm.range);
    EXPECT_EQ(27U,            sm.mesh.points.size());
    EXPECT_EQ(50U,            sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, CylinderX2) {
    const TriMesh mesh = BuildCylinderMesh(4, 8, 20, 8);
    const std::vector<float> fractions{.2f, .75f};
    SlicedMesh sm = SliceMesh(mesh, Dim::kX, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateAndRepairTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kX,        sm.axis);
    EXPECT_EQ(Range1f(-8, 8), sm.range);
    EXPECT_EQ(38U,            sm.mesh.points.size());
    EXPECT_EQ(72U,            sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, CylinderX3) {
    const TriMesh mesh = BuildCylinderMesh(4, 8, 20, 8);
    const std::vector<float> fractions{.2f, .6f, .75f};
    SlicedMesh sm = SliceMesh(mesh, Dim::kX, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateAndRepairTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kX,        sm.axis);
    EXPECT_EQ(Range1f(-8, 8), sm.range);
    EXPECT_EQ(50U,            sm.mesh.points.size());
    EXPECT_EQ(96U,            sm.mesh.GetTriangleCount());
}

TEST_F(MeshSlicingTest, CylinderX3b) {
    const TriMesh mesh = BuildCylinderMesh(4, 8, 20, 20);
    const std::vector<float> fractions{.2f, .6f, .75f};
    SlicedMesh sm = SliceMesh(mesh, Dim::kX, fractions);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ValidateAndRepairTriMesh(sm.mesh));
    EXPECT_EQ(Dim::kX,        sm.axis);
    EXPECT_EQ(Range1f(-8, 8), sm.range);
    EXPECT_EQ(104U,           sm.mesh.points.size());
    EXPECT_EQ(204U,           sm.mesh.GetTriangleCount());
}
