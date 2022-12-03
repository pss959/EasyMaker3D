#include "Debug/Dump3dv.h"
#include "Math/Bevel.h"
#include "Math/Beveler.h" // XXXX
#include "Math/Beveler2.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "TestBase.h"
#include "Testing.h"

class Beveler2Test : public TestBase {
  protected:
    // Beveler2Test() { EnableKLog("l"); }  // XXXX For debugging.

    // TEMPORARY!!!
    static constexpr float kScale = 2;

    /// Returns a default Bevel.
    static Bevel GetDefaultBevel() {
        Bevel bevel;
        // XXXX Bevel all edges for now...
        bevel.max_angle = Anglef::FromDegrees(180);
        bevel.scale     = kScale;  // XXXX
        return bevel;
    }

    /// Returns a Bevel with the given number of point and optional scale.
    static Bevel GetBevel(size_t np, float scale = kScale) {
        ASSERT(np <= 6U);
        Bevel bevel;
        // XXXX Bevel all edges for now...
        bevel.max_angle = Anglef::FromDegrees(180);
        bevel.scale = 1; // XXXX scale;  // XXXX
        if (np == 3) {
            bevel.profile.AddPoint(Point2f(.4, .8));
        }
        else if (np == 4) {
            bevel.profile.AddPoint(Point2f(.4, .8));
            bevel.profile.AddPoint(Point2f(.9, .2));
        }
        else if (np == 5) {
            bevel.profile.AddPoint(Point2f(.4, .8));
            bevel.profile.AddPoint(Point2f(.6, .6));
            bevel.profile.AddPoint(Point2f(.9, .2));
        }
        else if (np == 6) {
            bevel.profile.AddPoint(Point2f(.4, .9));
            bevel.profile.AddPoint(Point2f(.5, .8));
            bevel.profile.AddPoint(Point2f(.8, .6));
            bevel.profile.AddPoint(Point2f(.9, .3));
        }
        return bevel;
    }
};

// XXXX Fix all of these to test result meshes.
// XXXX Move common code into base class.

TEST_F(Beveler2Test, BevelBox) {
    // Create a 10x20x10 box TriMesh.
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));

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
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    const TriMesh rm = Beveler2::ApplyBevel(m, GetBevel(3));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}

TEST_F(Beveler2Test, BevelBox4Points) {
    // Create a 10x20x10 box TriMesh.
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    const TriMesh rm = Beveler2::ApplyBevel(m, GetBevel(4));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}
TEST_F(Beveler2Test, BevelBox4PointsOLD) {  // XXXX
    // Create a 10x20x10 box TriMesh.
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    const TriMesh rm = Beveler::ApplyBevel(m, GetBevel(4));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}

TEST_F(Beveler2Test, BevelBox5Points) {
    // Create a 10x20x10 box TriMesh.
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    const TriMesh rm = Beveler2::ApplyBevel(m, GetBevel(5));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}

TEST_F(Beveler2Test, BevelBox6Points) {
    // Create a 10x20x10 box TriMesh.
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    const TriMesh rm = Beveler2::ApplyBevel(m, GetBevel(6));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}
TEST_F(Beveler2Test, BevelBox6PointsOLD) {
    // Create a 10x20x10 box TriMesh.
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    const TriMesh rm = Beveler::ApplyBevel(m, GetBevel(6));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
}

TEST_F(Beveler2Test, BevelCyl) {
    // Create a 10x20x10 cylinder TriMesh.
    const TriMesh m = BuildCylinderMesh(5, 5, 20, 7);
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled box");
}

TEST_F(Beveler2Test, BevelTextO) {
    const TriMesh m = LoadTriMesh("O.stl");
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled text O");
}

TEST_F(Beveler2Test, BevelTetrahedron) {
    const TriMesh m = BuildTetrahedronMesh(10);
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // XXXX EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelPyramid) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    const TriMesh m = BuildCylinderMesh(0, 10, 20, 4);
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelPyramid3Points) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    const TriMesh m = BuildCylinderMesh(0, 10, 20, 4);
    const TriMesh rm = Beveler2::ApplyBevel(m, GetBevel(3));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelPyramid4Points) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    const TriMesh m = BuildCylinderMesh(0, 10, 20, 4);
    const TriMesh rm = Beveler2::ApplyBevel(m, GetBevel(4));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}
TEST_F(Beveler2Test, BevelPyramid4PointsOLD) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    const TriMesh m = BuildCylinderMesh(0, 10, 20, 4);
    const TriMesh rm = Beveler::ApplyBevel(m, GetBevel(4));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelPyramid6Points) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    const TriMesh m = BuildCylinderMesh(0, 10, 20, 4);
    const TriMesh rm = Beveler::ApplyBevel(m, GetBevel(6));
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}

TEST_F(Beveler2Test, BevelComplex) {
    const TriMesh m = LoadTriMesh("double_helix.stl");
    const TriMesh rm = Beveler2::ApplyBevel(m, GetDefaultBevel());
    EXPECT_LT(0U, rm.GetTriangleCount());  // XXXX
    // EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    // XXXX ValidateMesh(rm, "Beveled complex model");
}
