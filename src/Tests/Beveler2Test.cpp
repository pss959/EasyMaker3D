#include "Debug/Dump3dv.h"
#include "Math/Bevel.h"
#include "Math/Beveler2.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "TestBase.h"
#include "Testing.h"

class Beveler2Test : public TestBase {
  protected:
    // Beveler2Test() { EnableKLog("l"); }  // XXXX For debugging.

    /// Returns a default Bevel.
    static Bevel GetDefaultBevel() {
        Bevel bevel;
        // XXXX Bevel all edges for now...
        bevel.max_angle = Anglef::FromDegrees(180);
        bevel.scale     = .25f;  // XXXX
        return bevel;
    }
};

// XXXX Fix all of these to test result meshes.
// XXXX Move common code into base class.

TEST_F(Beveler2Test, BevelBox) {
    // Create a 10x20x10 box TriMesh.
    TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());

    // Beveling each edge of the cube results in:
    //    6 new faces with 4 new vertices each      (2 tris per)
    //   12 new rectangular faces for the edges     (2 tris per)
    //    8 new triangular  faces for the vertices  (1 tri per)
    // Total of 6*4=24 vertices and 6*2+12*2+8*1=44 triangles.
    // EXPECT_EQ(24U, rm.points.size());
    // EXPECT_EQ(44U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled box");
}

TEST_F(Beveler2Test, BevelBox3Points) {
    // Create a 10x20x10 box TriMesh.
    TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));

    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.4, .8));
    bevel.max_angle = Anglef::FromDegrees(180);
    bevel.scale = .25f;

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, bevel);

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}

TEST_F(Beveler2Test, BevelBox4Points) {
    // Create a 10x20x10 box TriMesh.
    TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));

    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.4, .8));
    bevel.profile.AddPoint(Point2f(.6, .6));
    bevel.max_angle = Anglef::FromDegrees(180);
    bevel.scale = .25f;

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, bevel);

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}

TEST_F(Beveler2Test, BevelBox5Points) {
    // Create a 10x20x10 box TriMesh.
    TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));

    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.4, .8));
    bevel.profile.AddPoint(Point2f(.6, .6));
    bevel.profile.AddPoint(Point2f(.9, .2));
    bevel.max_angle = Anglef::FromDegrees(180);
    bevel.scale = .25f;

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, bevel);

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}


TEST_F(Beveler2Test, BevelBox6Points) {
    // Create a 10x20x10 box TriMesh.
    TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));

    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.4, .8));
    bevel.profile.AddPoint(Point2f(.6, .6));
    bevel.profile.AddPoint(Point2f(.8, .5));
    bevel.profile.AddPoint(Point2f(.9, .2));
    bevel.max_angle = Anglef::FromDegrees(180);
    bevel.scale = .25f;

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, bevel);

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}

TEST_F(Beveler2Test, BevelCyl) {
    // Create a 10x20x10 cylinder TriMesh.
    TriMesh m = BuildCylinderMesh(5, 5, 20, 7);

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());

    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled box");
}

TEST_F(Beveler2Test, BevelTextO) {
    TriMesh m = LoadTriMesh("O.stl");

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());

    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled text O");
}

TEST_F(Beveler2Test, BevelTetrahedron) {
    TriMesh m = BuildTetrahedronMesh(10);

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX

    // XXXX EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelPyramid) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    TriMesh m = BuildCylinderMesh(0, 10, 20, 4);

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelPyramid3Points) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    TriMesh m = BuildCylinderMesh(0, 10, 20, 4);

    // Apply the bevel.
    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.4, .8));
    bevel.max_angle = Anglef::FromDegrees(180);
    bevel.scale = .25f;
    const TriMesh rm = Beveler2::ApplyBevel(m, bevel);

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelPyramid4Points) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    TriMesh m = BuildCylinderMesh(0, 10, 20, 4);

    // Apply the bevel.
    Bevel bevel;
    bevel.profile.AddPoint(Point2f(.4, .8));
    bevel.profile.AddPoint(Point2f(.6, .6));
    bevel.max_angle = Anglef::FromDegrees(180);
    bevel.scale = .25f;
    const TriMesh rm = Beveler2::ApplyBevel(m, bevel);

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelComplex) {
    TriMesh m = LoadTriMesh("double_helix.stl");

    // Apply the bevel.
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());

    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}
