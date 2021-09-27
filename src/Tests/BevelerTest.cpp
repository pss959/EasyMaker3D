#include "Debug/Dump3dv.h"
#include "Math/Beveler.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/PolyMeshMerging.h"
#include "Testing.h"

class BevelerTest : public TestBase {
  protected:
    // This can be useful to debug problems. It dumps a 3dv file at every step.
    void DumpBevel(const TriMesh &m, const Bevel &bevel,
                   const std::string &prefix);
};

void BevelerTest::DumpBevel(const TriMesh &m, const Bevel &bevel,
                            const std::string &prefix) {
    Dump3dv::DumpTriMesh(m, "Original " + prefix + " as TriMesh",
                         "/tmp/" + prefix + "0.3dv", .3f, false);

    PolyMesh pm(m);
    Dump3dv::DumpPolyMesh(pm, "Original " + prefix + " as PolyMesh",
                          "/tmp/" + prefix + "1.3dv", .3f,
                          Dump3dv::kVertexLabels);

    MergeCoplanarFaces(pm);
    Dump3dv::DumpPolyMesh(pm, "Merged " + prefix + " PolyMesh",
                          "/tmp/" + prefix + "2.3dv", .3f,
                          Dump3dv::kVertexLabels);

    PolyMesh bpm = Beveler::ApplyBevel(pm, bevel);
    Dump3dv::DumpPolyMesh(bpm, "Beveled " + prefix + " PolyMesh",
                          "/tmp/" + prefix + "3.3dv", .3f,
                          Dump3dv::kVertexLabels);
}

TEST_F(BevelerTest, BevelBox) {
    TriMesh m = BuildBoxMesh(Vector3f(10, 10, 10));

    // Apply the default Bevel.
    TriMesh rm = Beveler::ApplyBevel(m, Bevel());

    // Beveling each edge of the cube results in:
    //    6 new faces with 4 new vertices each      (2 tris per)
    //   12 new rectangular faces for the edges     (2 tris per)
    //    8 new triangular  faces for the vertices  (1 tri per)
    // Total of 6*4=24 vertices and 6*2+12*2+8*1=44 triangles.
    EXPECT_EQ(24U, rm.points.size());
    EXPECT_EQ(44U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled box");
}


TEST_F(BevelerTest, BevelBox3Pts) {
    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.5f, .6f));
    TriMesh m = BuildBoxMesh(Vector3f(10, 10, 10));
    TriMesh rm = Beveler::ApplyBevel(m, bevel);
    EXPECT_EQ(48U, rm.points.size());
    EXPECT_EQ(92U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled box");
}

TEST_F(BevelerTest, BevelConcave) {
    TriMesh m = LoadTriMesh("L.stl");
    TriMesh rm = Beveler::ApplyBevel(m, Bevel());
    EXPECT_EQ(36U, rm.points.size());
    EXPECT_EQ(68U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled L");
}

TEST_F(BevelerTest, BevelHole) {
    // This is a 20x20x20 box with a 10x10 hole from top to bottom.
    TriMesh m = LoadTriMesh("hole.stl");
    TriMesh rm = Beveler::ApplyBevel(m, Bevel());
    EXPECT_EQ(48U, rm.points.size());
    EXPECT_EQ(96U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled hole");
}

#if XXXX  // This test is disabled because it fails. Need to fix it.
TEST_F(BevelerTest, BevelClippedCyl) {
    TriMesh m = LoadTriMesh("clippedCyl.stl");
    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.5f, .6f));
    TriMesh rm = Beveler::ApplyBevel(m, bevel);
    EXPECT_EQ(48U, rm.points.size());
    EXPECT_EQ(84U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled clipped cylinder");
}
#endif

#if XXXX
    [Test]
    public void BevelHoleO() {
        FontManager mgr = new FontManager();
        List<Polygon> polys = FreeTypeWrapper.LayOutText(
            mgr.GetDefaultFontDesc().path, "O", Defaults.ModelComplexity,
            new TextOptions());

        Mesh em = MeshEditor.Extrude(polys[0], 2f);
        ValidateMesh(em, "Extruded O");

        // Use a small bevel scale so the mesh is not self-intersecting.
        BevelInfo bevel = new BevelInfo();
        bevel.scale = .1f;
        Mesh pm = MeshEditor.ApplyBevel(em, bevel);
        ValidateMesh(pm, "Beveled O");
    }
}
#endif
