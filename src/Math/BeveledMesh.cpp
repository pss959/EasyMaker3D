#include "Math/BeveledMesh.h"

#include <algorithm>

#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/Polygon.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Skeleton.h"
#include "Util/General.h"

#include "Debug/Dump3dv.h" // XXXX

// ----------------------------------------------------------------------------
// Helper_ class.
// ----------------------------------------------------------------------------

/// XXXX Class to help build BeveledMesh.
class Helper_ {
  public:
    void AddFace(const PolyMesh::Face &face,
                 std::vector<Point3f> &skel_vertices,
                 std::vector<size_t>  &skel_edges);
};

void Helper_::AddFace(const PolyMesh::Face &face,
                      std::vector<Point3f> &skel_vertices,
                      std::vector<size_t>  &skel_edges) {
    // Access all of the vertices and border counts for the face.
    PolyMesh::VertexVec vertices;
    std::vector<size_t> border_counts;
    PolyMesh::GetFaceVertices(face, vertices, border_counts);
    std::cerr << "XXXX For face " << face.ToString() << ":\n";
    for (size_t i = 0; i < vertices.size(); ++i)
        std::cerr << "XXXX    V [" << i << "] = " << vertices[i]->point << "\n";

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

    // Get the two dimensions for projecting.
    int dim0 = (max_index + 1) % 3;
    int dim1 = (max_index + 2) % 3;
    if (plane_flipped)
        std::swap(dim0, dim1);

    const Rotationf rot = Rotationf::RotateInto(normal, plane_normal);
    std::vector<Point2f> points;
    points.reserve(vertices.size());
    for (const auto &v: vertices) {
        const Point3f p = rot * v->point;
        points.push_back(Point2f(p[dim0], p[dim1]));
    }

    Polygon polygon(points, border_counts);

    polygon.Dump("XXXX " + face.ToString());

    // Compute the offset polygon for the face.
    std::vector<Point2f> svertices;
    std::vector<size_t>  bisectors;
    ComputeSkeleton(polygon, svertices, bisectors);
    std::cerr << "XXXX For face " << face.ToString() << ":\n";
    for (size_t i = 0; i < svertices.size(); ++i)
        std::cerr << "XXXX    V [" << i << "] = " << svertices[i] << "\n";
    for (size_t i = 0; i < bisectors.size(); i += 2)
        std::cerr << "XXXX    B from " << bisectors[i]
                  << " to " << bisectors[i+1] << "\n";

    // Convert the skeleton points back to 3D.
    const float dist = vertices[0]->point[max_index];
    const auto to_3d = [&](const Point2f &p2){
        Point3f p3;
        p3[max_index] = dist;
        p3[dim0] = p2[0];
        p3[dim1] = p2[1];
        std::cerr << "XXXX ##### " << p2 << " => " << ((-rot) * p3) << "\n";
        return (-rot) * p3;
    };

    // Append the skeleton vertices and edges to the BeveledMesh. Vertex
    // indices need to be offset.
    const size_t vertex_offset = skel_vertices.size();
    Util::AppendVector(Util::ConvertVector<Point3f, Point2f>(svertices, to_3d),
                       skel_vertices);
    for (auto &b: bisectors)
        b += vertex_offset;
    Util::AppendVector(bisectors, skel_edges);
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
        helper.AddFace(*face, skel_vertices_, skel_edges_);
    }

    Debug::Dump3dv dump("/tmp/BMESH.3dv", "BeveledMesh");
    dump.SetLabelFontSize(60);
    //dump.SetExtraPrefix("T_");
    //dump.SetLabelOffset(Vector3f(1, 1, .1f));
    //dump.AddTriMesh(mesh);
    dump.SetExtraPrefix("M_");
    dump.AddPolyMesh(poly_mesh);
    dump.SetExtraPrefix("S_");
    dump.SetLabelOffset(Vector3f(1, 1, .1f));
    dump.AddEdges(skel_vertices_, skel_edges_);

    // XXXX Do something!

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    PolyMeshBuilder pmb;

    // Convert back to a TriMesh.
    result_mesh_ = poly_mesh.ToTriMesh();
}
