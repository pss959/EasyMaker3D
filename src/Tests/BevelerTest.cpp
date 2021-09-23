#include "Math/Beveler.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Testing.h"

class BevelerTest : public TestBase {};

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

#if XXXX
    [Test]
    public void BevelConcave() {
        // L-shaped model.
        Mesh m  = LoadMesh("TestFiles/STL/L.stl");
        Mesh pm = MeshEditor.ApplyBevel(m, new BevelInfo());

        CompareBounds(m.bounds, pm.bounds);
        ValidateMesh(pm, "Beveled L");
    }

    [Test]
    public void BevelHole() {
        // This is a 20x20x20 box with a 10x10 hole from top to bottom.
        Mesh m  = LoadMesh("TestFiles/STL/hole.stl");
        Mesh pm = MeshEditor.ApplyBevel(m, new BevelInfo());

        CompareBounds(m.bounds, pm.bounds);
        ValidateMesh(pm, "Beveled hole");
    }

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

    [Test]
    public void BevelClippedCyl() {
        Mesh m = LoadMesh("TestFiles/STL/clippedCyl.stl");
        BevelInfo bevel = new BevelInfo();
        bevel.profile.points.Add(UT.Vec2(.5f, .6f));
        Mesh pm = MeshEditor.ApplyBevel(m, bevel);
        ValidateMesh(pm, "Beveld clipped cylinder");
    }
}
#endif
