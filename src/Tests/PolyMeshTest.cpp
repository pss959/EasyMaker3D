#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshMerging.h"
#include "Testing.h"
#include "Util/Read.h"

class PolyMeshTest : public TestBase {
  protected:
    // Loads a TriMesh from a file.
    TriMesh LoadTriMesh(const std::string &file_name) {
        const Util::FilePath path = GetDataPath(file_name);
        ion::gfx::ShapePtr shape = Util::ReadShape(path, false, false);
        ASSERTM(shape, "Loaded from " + path.ToString());
        return IonShapeToTriMesh(*shape);
    }
};

TEST_F(PolyMeshTest, Box) {
    TriMesh m = BuildBoxMesh(Vector3f(10, 10, 10));
    PolyMesh poly_mesh(m);
    TriMesh rm = poly_mesh.ToTriMesh();

    EXPECT_EQ(8U,  rm.points.size());
    EXPECT_EQ(12U, rm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(rm));
    ValidateMesh(rm, "Mesh from PolyMesh");
}

TEST_F(PolyMeshTest, MergeCoplanarFacesCyl3) {
    // Tests merging multiple triangles on 3-sided cylinder.  Merging leaves no
    // extra vertices: 1 triangle on top and 2 for each of 3 sides = 8
    // triangles.
    TriMesh m = BuildCylinderMesh(10, 10, 20, 3);
    PolyMesh poly_mesh(m);
    MergeCoplanarFaces(poly_mesh);
    TriMesh mm = poly_mesh.ToTriMesh();
    EXPECT_EQ(6U, mm.points.size());
    EXPECT_EQ(8U, mm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(mm));
    ValidateMesh(mm, "Merged cyl3");
}

TEST_F(PolyMeshTest, MergeCoplanarFacesCyl4) {
    // 4-sided cylinder: merging leaves 2 triangles on top and bottom and 2 on
    // each of 4 sides = 12 triangles.
    TriMesh m = BuildCylinderMesh(10, 10, 20, 4);
    PolyMesh poly_mesh(m);
    MergeCoplanarFaces(poly_mesh);
    TriMesh mm = poly_mesh.ToTriMesh();
    EXPECT_EQ(8U,  mm.points.size());
    EXPECT_EQ(12U, mm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(mm));
    ValidateMesh(mm, "Merged cyl4");
}

#if XXXX
TEST_F(PolyMeshTest, MergeCoplanarFacesConcave) {
    // 4-sided cylinder: merging leaves 2 triangles on top and bottom and 2 on
    // each of 4 sides = 12 triangles.
    TriMesh m = LoadTriMesh("L.stl");
    PolyMesh poly_mesh(m);
    MergeCoplanarFaces(poly_mesh);
    TriMesh mm = poly_mesh.ToTriMesh();
    EXPECT_EQ(m.points.size(),      mm.points.size());
    EXPECT_EQ(m.GetTriangleCount(), mm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(mm));
    ValidateMesh(mm, "Merged L.stl");
}

    [Test]
    public void MergeCoplanarFacesConcave() {
        // Solid L-shaped model with 2 concave sides that need to be
        // triangulated carefully.
        Mesh m = LoadMesh("TestFiles/STL/L.stl");
        Solid solid = new Solid(m);
        MeshEditor.MergeCoplanarFacesForTesting(solid);
        Mesh mm = MeshEditor.ToUnityMesh(solid);
        Assert.AreEqual(m.vertices.Length,  mm.vertices.Length);
        Assert.AreEqual(m.triangles.Length, mm.triangles.Length);
        CompareBounds(m.bounds, mm.bounds);
        ValidateMesh(mm, "Merged L");
    }

    [Test]
    public void MergeCoplanarFacesHole() {
        // This came from a 3D text "O".
        Mesh m = LoadMesh("TestFiles/STL/O.stl");
        Solid solid = new Solid(m);

        // The outer ring of the "O" has 11 vertices and the inner ring has 9
        // vertices, both on the top and bottom.
        Assert.AreEqual(2 * (11 + 9), solid.vertices.Count);

        // There are 20 triangular faces in top/bottom rings. There are 9 quads
        // in the inner sides, and 11 quads on the outer sides, where each quad
        // is 2 trianglular faces.
        Assert.AreEqual(2 * 20 + 2 * (9 + 11), solid.faces.Count);

        // There are 18 triangles forming the top and bottom rings
        MeshEditor.MergeCoplanarFacesForTesting(solid);

        // After merging, there should be no new vertices.
        Assert.AreEqual(2 * (11 + 9), solid.vertices.Count);

        // The top and bottom rings should each be a single face, and each side
        // quad should be a single face.
        Assert.AreEqual(2 + 9 + 11, solid.faces.Count);

        // Get the top and bottom rings, using the normals as a selector.
        Face top = solid.faces.Find(f => f.GetNormal() ==  Vector3.up);
        Face bot = solid.faces.Find(f => f.GetNormal() == -Vector3.up);
        TestRing(top, "top");
        TestRing(bot, "bottom");

        // Validates the top or bottom ring face.
        void TestRing(Face f, string which) {
            Assert.IsNotNull(f, which);

            // Should have 11 outer edges and a hole with 9 edges.
            Assert.AreEqual(11, f.edges.Count, which);
            Assert.AreEqual(1,  f.holeEdges.Count, which);
            Assert.AreEqual(9,  f.holeEdges[0].Count, which);
        }
    }
#endif
