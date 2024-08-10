#include "Math/Bounds.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshMerging.h"
#include "Math/TriMesh.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class PolyMeshTest : public TestBase {};

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

TEST_F(PolyMeshTest, MergeCoplanarFacesConcave) {
    TriMesh m = LoadTriMesh("L.stl");
    PolyMesh poly_mesh(m);
    MergeCoplanarFaces(poly_mesh);
    TriMesh mm = poly_mesh.ToTriMesh();
    EXPECT_EQ(m.points.size(),      mm.points.size());
    EXPECT_EQ(m.GetTriangleCount(), mm.GetTriangleCount());
    EXPECT_EQ(ComputeMeshBounds(m), ComputeMeshBounds(mm));
    ValidateMesh(mm, "Merged L.stl");
}

TEST_F(PolyMeshTest, MergeCoplanarFacesHole) {
    // This came from a 3D text "O".
    TriMesh m = LoadTriMesh("O.stl");
    PolyMesh poly_mesh(m);

    // The outer ring of the "O" has 11 vertices and the inner ring has 9
    // vertices, both on the top and bottom.
    EXPECT_EQ(2U * (11U + 9U), poly_mesh.vertices.size());

    // There are 20 triangular faces in top/bottom rings. There are 9 quads in
    // the inner sides, and 11 quads on the outer sides, where each quad is 2
    // trianglular faces.
    EXPECT_EQ(2U * 20U + 2U * (9U + 11U), poly_mesh.faces.size());

    MergeCoplanarFaces(poly_mesh);

    // After merging, there should be no new vertices.
    EXPECT_EQ(2U * (11U + 9U), poly_mesh.vertices.size());

    // The top and bottom rings should each be a single face, and each side
    // quad should be a single face.
    EXPECT_EQ(2U + 9U + 11U, poly_mesh.faces.size());

    // Get the top and bottom rings, using the normals as a selector.
    auto finder = [&poly_mesh](const Vector3f &n){
        auto it = std::find_if(poly_mesh.faces.begin(), poly_mesh.faces.end(),
                               [&n](const PolyMesh::Face *f){
                               return f->GetNormal() == n; });
        EXPECT_NE(poly_mesh.faces.end(), it);
        return **it;
    };

    const PolyMesh::Face &top = finder(Vector3f(0,  1, 0));
    const PolyMesh::Face &bot = finder(Vector3f(0, -1, 0));

    // Each should have 11 outer edges and a hole with 9 edges.
    EXPECT_EQ(11U, top.outer_edges.size());
    EXPECT_EQ(1U,  top.hole_edges.size());
    EXPECT_EQ(9U,  top.hole_edges[0].size());
    EXPECT_EQ(11U, bot.outer_edges.size());
    EXPECT_EQ(1U,  bot.hole_edges.size());
    EXPECT_EQ(9U,  bot.hole_edges[0].size());
}

TEST_F(PolyMeshTest, GetFaceVertices) {
    // Use a mesh with a hole for completeness.
    TriMesh m = LoadTriMesh("O.stl");
    PolyMesh poly_mesh(m);
    MergeCoplanarFaces(poly_mesh);
    for (const auto &face: poly_mesh.faces) {
        PolyMesh::VertexVec verts;
        std::vector<size_t> border_counts;
        PolyMesh::GetFaceVertices(*face, verts, border_counts);
        // The faces with holes have 20 vertices and border counts [11, 9].
        // All other faces have 4 vertices and one border.
        EXPECT_TRUE(verts.size() == 4U || verts.size() == 20U);
        if (verts.size() == 4U) {
            EXPECT_EQ(1U, border_counts.size());
            EXPECT_EQ(4U, border_counts[0]);
        }
        else {
            EXPECT_EQ(2U,  border_counts.size());
            EXPECT_EQ(11U, border_counts[0]);
            EXPECT_EQ(9U,  border_counts[1]);
        }
    }

    // Merging again should have no effect.
    MergeCoplanarFaces(poly_mesh);
    for (const auto &face: poly_mesh.faces) {
        PolyMesh::VertexVec verts;
        std::vector<size_t> border_counts;
        PolyMesh::GetFaceVertices(*face, verts, border_counts);
        // The faces with holes have 20 vertices and border counts [11, 9].
        // All other faces have 4 vertices and one border.
        EXPECT_TRUE(verts.size() == 4U || verts.size() == 20U);
        if (verts.size() == 4U) {
            EXPECT_EQ(1U, border_counts.size());
            EXPECT_EQ(4U, border_counts[0]);
        }
        else {
            EXPECT_EQ(2U,  border_counts.size());
            EXPECT_EQ(11U, border_counts[0]);
            EXPECT_EQ(9U,  border_counts[1]);
        }
    }
}

