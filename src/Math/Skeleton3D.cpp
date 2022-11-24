#include "Math/Skeleton3D.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/PolyMesh.h"
#include "Math/Polygon.h"
#include "Math/Skeleton2D.h"
#include "Util/Assert.h"
#include "Util/General.h"

// ----------------------------------------------------------------------------
// Skeleton3D::Helper_ class.
// ----------------------------------------------------------------------------

class Skeleton3D::Helper_ {
  public:
    explicit Helper_(const PolyMesh &mesh);

    const std::vector<Vertex> & GetVertices() const { return vertices_; }
    const std::vector<Edge>   & GetEdges()    const { return edges_; }

  private:
    /// Struct storing everything needed for conversion between 3D and 2D, and
    /// vice-versa.
    struct DimConversion_ {
        Rotationf rot;     ///< Rotation from 3D plane to principal plane.
        float     offset;  ///< Offset of rotated plane from principal plane.
        int       pdim;    ///< Dimension of principal plane.
        int       dim0;    ///< First other dimension.
        int       dim1;    ///< Second other dimension.
    };

    typedef std::unordered_map<const PolyMesh::Vertex *, size_t> PMVertexMap_;

    std::vector<Vertex> vertices_;  ///< Resulting skeleton vertices.
    std::vector<Edge>   edges_;     ///< Resulting skeleton edges.

    /// Adds one Face of a PolyMesh. The PMVertexMap_ is used to determine the
    /// correct source_index to use for each added Vertex.
    void AddFace_(const PolyMesh::Face &face,
                  const PMVertexMap_ &pm_vertex_map);

    /// Stores a DimConversion_ instance that is used to convert between
    /// dimensions for a PolyMesh::Face.
    void GetDimConversion_(const PolyMesh::Face &face, DimConversion_ &dc);

    /// Converts vertices of a PolyMesh::Face to a Polygon by rotating the
    /// points to the closest principal plane and removing the appropriate
    /// dimension. The DimConversion_ supplies all needed values.
    Polygon FaceToPolygon_(const PolyMesh::VertexVec &face_vertices,
                           const std::vector<size_t> &border_counts,
                           const DimConversion_ &dc);

    /// Converts vertices of a Skeleton2D to 3D using a DimConversion_.
    std::vector<Vertex> To3DVertices_(const Skeleton2D &skel2,
                                      const DimConversion_ &dc);

    /// Removes duplicate Vertex instances from the vertices_ vector. This
    /// stores the resulting index for each original Vertex into the shrunken
    /// vertices_ vector in index_map.
    void RemoveDuplicateVertices_(size_t source_index_count,
                                  std::vector<size_t> &index_map);
};

// ----------------------------------------------------------------------------
// Skeleton3D::Helper_ functions.
// ----------------------------------------------------------------------------

Skeleton3D::Helper_::Helper_(const PolyMesh &mesh) {
    // Store correspondences from all PolyMesh::Vertex pointers to their
    // indices.
    PMVertexMap_ pm_vertex_map;
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
        pm_vertex_map[mesh.vertices[i]] = i;

    // Add all faces to the skeleton vertices and edges.
    for (const auto &face: mesh.faces)
        AddFace_(*face, pm_vertex_map);

    // Remove duplicate vertices and get a map from old vertex index to new.
    std::vector<size_t> vertex_index_map;
    RemoveDuplicateVertices_(mesh.vertices.size(), vertex_index_map);

    // Store positions of all source vertices. Doing this provides more
    // accuracy than using rotated 2D vertices.
    for (auto &v: vertices_)
        if (v.source_index >= 0)
            v.point = mesh.vertices[v.source_index]->point;

    // Update vertex indices in all edges.
    for (auto &edge: edges_) {
        edge.v0_index = vertex_index_map[edge.v0_index];
        edge.v1_index = vertex_index_map[edge.v1_index];
        if (edge.bisected_index0 >= 0)
            edge.bisected_index0 = vertex_index_map[edge.bisected_index0];
        if (edge.bisected_index1 >= 0)
            edge.bisected_index1 = vertex_index_map[edge.bisected_index1];
    }
}

void Skeleton3D::Helper_::AddFace_(const PolyMesh::Face &face,
                                   const PMVertexMap_ &pm_vertex_map) {
    // Access all of the vertices and border counts for the face.
    PolyMesh::VertexVec face_vertices;
    std::vector<size_t> border_counts;
    PolyMesh::GetFaceVertices(face, face_vertices, border_counts);

    // Fill in a DimConversion_ for the face.
    DimConversion_ dc;
    GetDimConversion_(face, dc);

    // Convert the face vertices to a Polygon.
    const Polygon polygon = FaceToPolygon_(face_vertices, border_counts, dc);

    // Compute the 2D skeleton for the polygon.
    Skeleton2D skel2;
    skel2.BuildForPolygon(polygon);

    // Convert the 2D skeleton vertices back to 3D vertices.
    std::vector<Vertex> skel_vertices = To3DVertices_(skel2, dc);

    // The source_index in each vertex is relative to the 2D Polygon. Fix it to
    // be relative to the 3D PolyMesh using the PMVertexMap_.
    for (auto &v: skel_vertices) {
        if (v.source_index >= 0)
            v.source_index = pm_vertex_map.at(face_vertices[v.source_index]);
    }

    // Append the 3D skeleton vertices after saving the previous size.
    const size_t vertex_offset = vertices_.size();
    Util::AppendVector(skel_vertices, vertices_);

    // Add edges, adding the offset to vertex indices.
    edges_.reserve(edges_.size() + skel2.GetEdges().size());
    for (const auto &edge2: skel2.GetEdges()) {
        Edge edge3;
        edge3.v0_index = edge2.v0_index + vertex_offset;
        edge3.v1_index = edge2.v1_index + vertex_offset;
        if (edge2.bisected_index0 >= 0)
            edge3.bisected_index0 = edge2.bisected_index0 + vertex_offset;
        if (edge2.bisected_index1 >= 0)
            edge3.bisected_index1 = edge2.bisected_index1 + vertex_offset;
        edges_.push_back(edge3);
    }
}

void Skeleton3D::Helper_::GetDimConversion_(const PolyMesh::Face &face,
                                            DimConversion_ &dc) {
    // Access the face normal and one 3D point on the face.
    const Vector3f &face_normal = face.GetNormal();
    const Point3f  &face_point  = face.outer_edges[0]->v0->point;

    // Use the largest (absolute) element of the face normal for the plane.
    dc.pdim = GetMaxAbsElementIndex(face_normal);

    const bool plane_flipped = face_normal[dc.pdim] < 0;
    const Vector3f plane_normal =
        plane_flipped ? -GetAxis(dc.pdim) : GetAxis(dc.pdim);

    // Compute the rotation that brings the face normal to the plane normal.
    dc.rot = Rotationf::RotateInto(face_normal, plane_normal);

    // Compute the offset of the plane.
    dc.offset = (dc.rot * face_point)[dc.pdim];

    // Get the other two dimensions for projecting.
    dc.dim0 = (dc.pdim + 1) % 3;
    dc.dim1 = (dc.pdim + 2) % 3;
    if (plane_flipped)
        std::swap(dc.dim0, dc.dim1);
}

Polygon Skeleton3D::Helper_::FaceToPolygon_(
    const PolyMesh::VertexVec &face_vertices,
    const std::vector<size_t> &border_counts, const DimConversion_ &dc) {
    // Rotate and project points onto the plane.
    std::vector<Point2f> points2d;
    points2d.reserve(face_vertices.size());
    for (const auto &fv: face_vertices) {
        const Point3f rp = dc.rot * fv->point;
        points2d.push_back(Point2f(rp[dc.dim0], rp[dc.dim1]));
    }
    return Polygon(points2d, border_counts);
}

std::vector<Skeleton3D::Vertex> Skeleton3D::Helper_::To3DVertices_(
    const Skeleton2D &skel2, const DimConversion_ &dc) {
    std::vector<Vertex> vertices;
    vertices.reserve(skel2.GetVertices().size());
    for (const auto &v2: skel2.GetVertices()) {
        // Offset the point in 3D and apply the inverse rotation.
        Point3f p3;
        p3[dc.pdim] = dc.offset;
        p3[dc.dim0] = v2.point[0];
        p3[dc.dim1] = v2.point[1];
        p3 = (-dc.rot) * p3;

        Vertex vertex;
        vertex.source_index = v2.source_index;   // Fixed later.
        vertex.point          = p3;
        vertex.distance       = v2.distance;
        vertices.push_back(vertex);
    }
    return vertices;
}

void Skeleton3D::Helper_::RemoveDuplicateVertices_(
    size_t source_index_count, std::vector<size_t> &index_map) {

    // Will contain only vertices that are not duplicates.
    std::vector<Vertex> result_vertices;
    result_vertices.reserve(vertices_.size());

    // Maps from original Vertex index to a Vertex in result_vertices.
    index_map.reserve(vertices_.size());

    // Used to indicate that no index was found yet.
    const size_t bad_index = vertices_.size();

    // Stores the index of the first Vertex with a specific source_index.
    std::vector<size_t> first_with_source_index(source_index_count, bad_index);

    for (size_t i = 0; i < vertices_.size(); ++i) {
        // First, determine if this is a duplicate vertex.
        const auto source_index = vertices_[i].source_index;
        const size_t orig_index = source_index < 0 ? bad_index :
            first_with_source_index[source_index];
        if (orig_index != bad_index) {
            // This is a duplicate of the Vertex at orig_index that was already
            // found for the source index.
            ASSERT(orig_index < index_map.size());
            index_map.push_back(index_map[orig_index]);
        }
        else {
            // This is either the first Vertex found for the source_index or
            // this was a Vertex created for the skeleton. If the former, store
            // in first_with_source_index. For either case, store the Vertex
            // and the index correspondence.
            if (source_index >= 0)
                first_with_source_index[source_index] = i;
            index_map.push_back(result_vertices.size());
            result_vertices.push_back(vertices_[i]);
        }
    }
    ASSERT(index_map.size() == vertices_.size());
    vertices_ = result_vertices;
}

// ----------------------------------------------------------------------------
// Skeleton3D functions.
// ----------------------------------------------------------------------------

void Skeleton3D::BuildForPolyMesh(const PolyMesh &mesh) {
    Helper_ helper(mesh);
    SetVertices(helper.GetVertices());
    SetEdges(helper.GetEdges());
}
