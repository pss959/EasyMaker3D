#include "Debug/Dump3dv.h"
#include "Math/BeveledMesh.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "TestBase.h"
#include "Testing.h"

class BeveledMeshTest : public TestBase {
};

// XXXX Fix all of these to test result meshes.

TEST_F(BeveledMeshTest, BevelBox) {
    // Create a 10x20x10 box TriMesh.
    TriMesh m = BuildBoxMesh(Vector3f(10, 20, 10));

    // Create a BeveledMesh applied to all edges.
    BeveledMesh bm(m, Anglef::FromDegrees(180));

    // Access and validate the resulting TriMesh.
    const TriMesh &rm = bm.GetResultMesh();

    // Beveling each edge of the cube results in:
    //    6 new faces with 4 new vertices each      (2 tris per)
    //   12 new rectangular faces for the edges     (2 tris per)
    //    8 new triangular  faces for the vertices  (1 tri per)
    // Total of 6*4=24 vertices and 6*2+12*2+8*1=44 triangles.
    // EXPECT_EQ(24U, rm.points.size());
    // EXPECT_EQ(44U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled box");
}

TEST_F(BeveledMeshTest, BevelCyl) {
    // Create a 10x20x10 cylinder TriMesh.
    TriMesh m = BuildCylinderMesh(5, 5, 20, 7);

    // Create a BeveledMesh applied to all edges.
    BeveledMesh bm(m, Anglef::FromDegrees(180));

    // Access and validate the resulting TriMesh.
    const TriMesh &rm = bm.GetResultMesh();

    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled box");
}

TEST_F(BeveledMeshTest, BevelTextO) {
    TriMesh m = LoadTriMesh("O.stl");

    // Create a BeveledMesh applied to all edges.
    BeveledMesh bm(m, Anglef::FromDegrees(180));

    // Access and validate the resulting TriMesh.
    const TriMesh &rm = bm.GetResultMesh();

    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Beveled text O");
}
