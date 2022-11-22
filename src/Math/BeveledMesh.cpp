#include "Math/BeveledMesh.h"

#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/Polygon.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Skeleton.h"

// ----------------------------------------------------------------------------
// Helper_ class.
// ----------------------------------------------------------------------------

/// XXXX Class to help build BeveledMesh.
class Helper_ {
  public:
    void AddFace(const PolyMesh::Face &face);
};

void Helper_::AddFace(const PolyMesh::Face &face) {
    // Access all of the vertices and border counts for the face.
    PolyMesh::VertexVec vertices;
    std::vector<size_t> border_counts;
    PolyMesh::GetFaceVertices(face, vertices, border_counts);

    // Convert the face to a Polygon by rotating the vertices to the closest
    // principal plane and removing the appropriate dimension.
    const Vector3f &normal = face.GetNormal();
    const int max_index = GetMaxAbsElementIndex(normal);
    Vector3f plane_normal = GetAxis(max_index);
    std::cerr << "XXXX index = " << max_index << " normal = " << normal
              << " plane_normal = " << plane_normal << "\n";
    const bool plane_flipped = normal[max_index] < 0;
    if (plane_flipped)
        plane_normal = -plane_normal;

    // Need to reverse coordinates in the following cases:
    //   - Projecting along -X.
    //   - Projecting along +Y.
    //   - Projecting along -Z.
    const bool reverse_coords =
        (max_index == 0 &&   plane_flipped) ||
        (max_index == 1 && ! plane_flipped) ||
        (max_index == 2 &&   plane_flipped);

    const Rotationf rot = Rotationf::RotateInto(normal, plane_normal);
    std::vector<Point2f> points;
    points.reserve(vertices.size());
    for (const auto &v: vertices) {
        const Point2f p = ToPoint2f(rot * v->point, max_index);
        points.push_back(reverse_coords ? Point2f(p[1], p[0]) : p);
    }

    Polygon polygon(points, border_counts);

    polygon.Dump("XXXX " + face.ToString());

    // Compute the offset polygon for the face.
    ComputeSkeleton(polygon);
}

// ----------------------------------------------------------------------------
// BeveledMesh functions.
// ----------------------------------------------------------------------------

BeveledMesh::BeveledMesh(const TriMesh &mesh, const Anglef &max_angle) {
    original_mesh_ = mesh;
    max_angle_     = max_angle;

    // Convert the original TriMesh to a PolyMesh and merge any coplanar faces.
    PolyMesh poly_mesh(mesh);
    MergeCoplanarFaces(poly_mesh);

    Helper_ helper;

    for (const auto &face: poly_mesh.faces) {
        helper.AddFace(*face);
    }

    // XXXX Do something!

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    PolyMeshBuilder pmb;

    // Convert back to a TriMesh.
    result_mesh_ = poly_mesh.ToTriMesh();
}
