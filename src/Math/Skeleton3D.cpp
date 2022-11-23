#include "Math/Skeleton3D.h"

#include <algorithm>

#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/PolyMesh.h"
#include "Math/Polygon.h"
#include "Math/Skeleton2D.h"
#include "Util/General.h"

void Skeleton3D::BuildForPolyMesh(const PolyMesh &mesh) {
    points_.clear();
    distances_.clear();
    edges_.clear();

    // Iterate over all faces of the PolyMesh.
    for (const auto &face: mesh.faces) {
        // Access all of the vertices and border counts for the face.
        PolyMesh::VertexVec face_vertices;
        std::vector<size_t> border_counts;
        PolyMesh::GetFaceVertices(*face, face_vertices, border_counts);

        // Access the points from the vertices.
        const std::vector<Point3f> face_points =
            Util::ConvertVector<Point3f, PolyMesh::Vertex *>(
                face_vertices,
                [](const PolyMesh::Vertex *v){ return v->point; });

        // Add the skeleton for the face to this.
        AddFace_(face_points, border_counts, face->GetNormal());
    }
}

void Skeleton3D::AddFace_(const std::vector<Point3f> &points,
                          const std::vector<size_t> &border_counts,
                          const Vector3f &normal) {
    // Convert the face to a Polygon by rotating the points to the closest
    // principal plane and removing the appropriate dimension.
    const int  max_index     = GetMaxAbsElementIndex(normal);
    Vector3f  plane_normal   = GetAxis(max_index);
    const bool plane_flipped = normal[max_index] < 0;
    if (plane_flipped)
        plane_normal = -plane_normal;

    // Get the two dimensions for projecting.
    int dim0 = (max_index + 1) % 3;
    int dim1 = (max_index + 2) % 3;
    if (plane_flipped)
        std::swap(dim0, dim1);

    const Rotationf rot = Rotationf::RotateInto(normal, plane_normal);
    std::vector<Point2f> points2d;
    points2d.reserve(points.size());
    for (const auto &p: points) {
        const Point3f rp = rot * p;
        points2d.push_back(Point2f(rp[dim0], rp[dim1]));
    }

    Polygon polygon(points2d, border_counts);

    // Get the distance of the points to the principal plane (along the
    // projected axis).
    const float dist = (rot * points[0])[max_index];

    // Compute the 2D skeleton for the face.
    Skeleton2D skel2;
    skel2.BuildForPolygon(polygon);

    // This is used to convert the skeleton points back to 3D.
    const auto to_3d = [&](const Point2f &p2){
        Point3f p3;
        p3[max_index] = dist;
        p3[dim0]      = p2[0];
        p3[dim1]      = p2[1];
        return (-rot) * p3;
    };

    // Append the 3D skeleton points, distances, and edges. Vertex indices used
    // for edges need to be offset.
    const size_t offset = points_.size();
    const auto add_offset = [offset](size_t i){ return i + offset; };
    Util::AppendVector(
        Util::ConvertVector<Point3f, Point2f>(skel2.GetPoints(), to_3d),
        points_);
    Util::AppendVector(skel2.GetDistances(), distances_);
    Util::AppendVector(
        Util::ConvertVector<size_t, size_t>(skel2.GetEdges(), add_offset),
        edges_);
}
