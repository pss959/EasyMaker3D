#include "Debug/Dump3dv.h"
#include "Math/Beveler.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "Math/PolyMeshMerging.h"
#include "TestBase.h"
#include "Testing.h"

class BevelerTest : public TestBase {
  protected:
    // This can be useful to debug problems. It dumps a 3dv file at every step.
    void DumpBevel(const TriMesh &m, const Bevel &bevel,
                   const std::string &prefix);
};

#if ENABLE_DEBUG_FEATURES
void BevelerTest::DumpBevel(const TriMesh &m, const Bevel &bevel,
                            const std::string &prefix) {
    using namespace Debug;

    std::string s, f;

    auto report = [&](){
        std::cerr << "*** Dumping " << s << " to '" << f << "'\n";
    };

    s = "Original " + prefix + " as TriMesh";
    f = "/tmp/" + prefix + "0.3dv";
    report();
    {
        Debug::Dump3dv dump(f, s);
        dump.SetLabelFontSize(10);
        dump.AddTriMesh(m);
    }

    PolyMesh pm(m);
    s = "Original " + prefix + " as PolyMesh";
    f = "/tmp/" + prefix + "1.3dv";
    report();
    {
        Debug::Dump3dv dump(f, s);
        dump.SetLabelFontSize(10);
        dump.AddPolyMesh(pm);
    }

    MergeCoplanarFaces(pm);
    s = "Merged " + prefix + " PolyMesh";
    f = "/tmp/" + prefix + "2.3dv";
    report();
    {
        Debug::Dump3dv dump(f, s);
        dump.SetLabelFontSize(10);
        dump.AddPolyMesh(pm);
    }

    PolyMesh bpm = Beveler::ApplyBevel(pm, bevel);
    s = "Beveled " + prefix + " PolyMesh";
    f = "/tmp/" + prefix + "3.3dv";
    report();
    {
        Debug::Dump3dv dump(f, s);
        dump.SetLabelFontSize(10);
        dump.AddPolyMesh(bpm);
    }

    TriMesh btm = bpm.ToTriMesh();
    s = "Beveled " + prefix + " TriMesh";
    f = "/tmp/" + prefix + "4.3dv";
    report();
    {
        Debug::Dump3dv dump(f, s);
        dump.SetLabelFontSize(10);
        dump.AddTriMesh(btm);
    }
}
#endif

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

TEST_F(BevelerTest, BevelBox2) {
    TriMesh m = BuildBoxMesh(Vector3f(4, 4, 4));

    // Construct a Bevel with 2 interior points.
    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.4, .8));
    bevel.profile.AddPoint(Point2f(.8, .4));

    // Apply the Bevel.
    TriMesh rm = Beveler::ApplyBevel(m, bevel);

    EXPECT_EQ(96U,  rm.points.size());
    EXPECT_EQ(188U, rm.GetTriangleCount());
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

TEST_F(BevelerTest, BevelClippedCyl) {
    TriMesh m = LoadTriMesh("clippedCyl.stl");
    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.5f, .6f));

    TriMesh rm = Beveler::ApplyBevel(m, bevel);
    EXPECT_EQ(42U, rm.points.size());
    EXPECT_EQ(80U, rm.GetTriangleCount());
    ValidateMesh(rm, "Beveled clipped cylinder");
}

TEST_F(BevelerTest, BevelTextO) {
    TriMesh m = LoadTriMesh("O.stl");

    // Scale of .6 works properly.
    Bevel bevel;
    bevel.scale = .6f;
    TriMesh rm = Beveler::ApplyBevel(m, bevel);
    EXPECT_EQ(80U,  rm.points.size());
    EXPECT_EQ(160U, rm.GetTriangleCount());
    ValidateMesh(rm, "Beveled text O");

    // Scale of .8 should cause CGAL problems.
    bevel.scale = .8f;
    rm = Beveler::ApplyBevel(m, bevel);
    const auto ret = ValidateTriMesh(rm);
    EXPECT_ENUM_EQ(MeshValidityCode::kInconsistent, ret);
}

TEST_F(BevelerTest, BevelTextA) {
    TriMesh m = LoadTriMesh("A.stl");

    // This caused and error with the original Beveler.
    Bevel bevel;
    bevel.scale = .1f;
    DumpBevel(m, bevel, "XXXX");

    TriMesh rm = Beveler::ApplyBevel(m, bevel);
    // XXXX EXPECT_EQ(80U,  rm.points.size());
    // XXXX EXPECT_EQ(160U, rm.GetTriangleCount());
    ValidateMesh(rm, "Beveled text A");
}
