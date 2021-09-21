#include "Math/PolyMesh.h"

#include <ion/math/vectorutils.h>

#include "Assert.h"
#include "Math/CGALInterface.h"
#include "Math/Linear.h"
#include "Math/Point3fMap.h"
#include "Util/General.h"

namespace {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

// Shorthand.
typedef PolyMesh::Edge   Edge;
typedef PolyMesh::Face   Face;
typedef PolyMesh::Vertex Vertex;

//! Converts a vector of Edge pointers to a vector of points using the V0
//! vertex of each edge.
static std::vector<Point3f> EdgesToPoints_(const PolyMesh::EdgeVec &edges) {
    return Util::ConvertVector<Point3f, Edge*>(
        edges, [](const Edge *e){ return e->GetV0().GetPoint(); });
}

//! Converts a vector of Edge pointers to a vector of Vertex pointers using
//! the V0 vertex of each edge.
static std::vector<const Vertex *> EdgesToVertices_(
    const PolyMesh::EdgeVec &edges) {
    return Util::ConvertVector<const Vertex*, Edge*>(
        edges, [](const Edge *e){ return &e->GetV0(); });
}

//! Projects points from vertices forming a face onto one of the Cartesian
//! planes, using the given normal to help choose a plane. Returns the
//! projected 2D points.
static std::vector<Point2f> ProjectPoints_(
    const std::vector<const Vertex *> vertices, const Vector3f &normal) {

    // Figure out which of the 3 Cartesian planes is best for projection by
    // using the largest component of the normal.
    const int dim = GetMaxAbsElementIndex(normal);

    // Get the two dimensions for projecting.
    const int dim0 = (dim + 1) % 3;
    const int dim1 = (dim + 2) % 3;

    // Project all the vertices onto that plane. If projecting along the
    // negative normal, negate one of the dimensions to create a clockwise
    // ordering.
    if (normal[dim] < 0) {
        auto func = [dim0, dim1](const Vertex *v){
            const Point3f &p = v->GetPoint();
            return Point2f(p[dim0], -p[dim1]);
        };
        return Util::ConvertVector<Point2f, const Vertex *>(vertices, func);
    }
    else {
        auto func = [dim0, dim1](const Vertex *v){
            const Point3f &p = v->GetPoint();
            return Point2f(p[dim0],  p[dim1]);
        };
        return Util::ConvertVector<Point2f, const Vertex *>(vertices, func);
    }
}

//! Returns a vector Vertex pointers representing triangles for one Face of a
//! PolyMesh.
static std::vector<const Vertex *> TriangulateFace_(const Face &face) {
    using ion::math::LengthSquared;

    // Collect vertices from all Face borders into one list. Keep track of
    // the size of each border added.
    std::vector<const Vertex *> vertices;
    std::vector<size_t>         border_counts;

    // Adds vertices from one border to vertices and the size to border_counts.
    auto add_border_verts =
        [&vertices, &border_counts](const PolyMesh::EdgeVec &edges){
        Util::AppendVector(EdgesToVertices_(edges), vertices);
        border_counts.push_back(edges.size()); };

    add_border_verts(face.GetOuterEdges());
    for (const auto &hole: face.GetHoleEdges())
        add_border_verts(hole);

    // Vertex indices of resulting triangles.
    std::vector<size_t> tri_indices;

    // Common triangulation case: no holes, 3 vertices.
    if (border_counts.size() == 1U && vertices.size() == 3U) {
        tri_indices.assign({ 0, 1, 2 });
    }
    // Another common triangulation case: no holes, 4 vertices.
    else if (border_counts.size() == 1U && vertices.size() == 4U) {
        // Split into two triangles by the shorter diagonal - this should work
        // for all quads, even concave ones.
        const float d02 = LengthSquared(vertices[2]->GetPoint() -
                                        vertices[0]->GetPoint());
        const float d13 = LengthSquared(vertices[3]->GetPoint() -
                                        vertices[1]->GetPoint());
        if (d02 <= d13)
            tri_indices.assign({ 0, 1, 2,  0, 2, 3 });
        else
            tri_indices.assign({ 1, 2, 3,  3, 0, 1 });
    }
    // General case: either there are holes or outer boundary has > 4 vertices.
    else {
        // Project all of the points into 2D based on the largest normal
        // component. Then triangulate using CGAL.
        const std::vector<Point2f> points2D =
            ProjectPoints_(vertices, face.GetNormal());
        tri_indices = TriangulatePolygon(Polygon(points2D, border_counts));
    }

    // Convert the indices to Vertex pointers.
    return Util::ConvertVector<const Vertex *, size_t>(
        tri_indices, [&vertices](const size_t i){ return vertices[i]; });
}

//! Returns a list of triangle indices representing all faces of a PolyMesh
// after triangulation.
static std::vector<size_t> GetTriangleIndices_(const PolyMesh &poly_mesh) {
    // This maps a Vertex pointer to its index in the full list of vertices for
    // the PolyMesh.
    std::unordered_map<const Vertex *, size_t> vertex_map;
    const std::vector<PolyMesh::Vertex> &vertices = poly_mesh.GetVertices();
    for (size_t i = 0; i < vertices.size(); ++i)
        vertex_map[&vertices[i]] = i;

    // Triangulate the vertices of each face, getting indices for each Vertex
    // triangle. Map these to original Vertex indices.
    std::vector<size_t> indices;
    for (const auto &face: poly_mesh.GetFaces()) {
        // Skip faces with zero area.
        const float area = ComputeArea(EdgesToPoints_(face.GetOuterEdges()));
        if (area > .001f) {
            std::vector<const Vertex *> tri_verts = TriangulateFace_(face);
            std::vector<size_t> tri_indices =
                Util::ConvertVector<size_t, const Vertex *>(
                    tri_verts,
                    [&vertex_map](const Vertex *v){
                    return vertex_map.find(v)->second; });
            Util::AppendVector(tri_indices, indices);
        }
    }
    return indices;
}

}   // anonymous namespace

// ----------------------------------------------------------------------------
// PolyMesh::Edge class functions.
// ----------------------------------------------------------------------------

Vector3f PolyMesh::Edge::GetUnitVector() const {
    return ion::math::Normalized(v1_.GetPoint() - v0_.GetPoint());
}

void PolyMesh::Edge::ConnectOpposite_(Edge &opposite) {
    // This should happen at most once.
    ASSERT(! opposite_edge_);
    ASSERT(! opposite.opposite_edge_);
    opposite_edge_          = &opposite;
    opposite.opposite_edge_ = this;
}

// ----------------------------------------------------------------------------
// PolyMesh::Face class functions.
// ----------------------------------------------------------------------------

const Vector3f & PolyMesh::Face::GetNormal() const {
    if (normal_ == Vector3f::Zero() && outer_edges_.size() >= 3U)
        normal_ = ComputeNormal(EdgesToPoints_(outer_edges_));
    return normal_;
}

void PolyMesh::Face::AddEdge_(int hole_index, Edge &edge) {
    if (hole_index < 0) {
        outer_edges_.push_back(&edge);
    }
    else {
        ASSERT(static_cast<size_t>(hole_index) < hole_edges_.size());
        hole_edges_[hole_index].push_back(&edge);
    }
}

// ----------------------------------------------------------------------------
// PolyMesh class functions.
// ----------------------------------------------------------------------------


PolyMesh::PolyMesh(const TriMesh &mesh) {
    vertices_.reserve(mesh.points.size());
    for (size_t i = 0; i < mesh.points.size(); ++i)
        vertices_.push_back(Vertex(i, mesh.points[i]));

    // Map used to find opposite edges.
    EdgeMap_ edge_map;

    // Build faces and edges.
    const size_t tri_count = mesh.GetTriangleCount();
    for (size_t t = 0; t < tri_count; ++t) {
        Vertex &v0 = vertices_[mesh.indices[3 * t + 0]];
        Vertex &v1 = vertices_[mesh.indices[3 * t + 1]];
        Vertex &v2 = vertices_[mesh.indices[3 * t + 2]];

        // Add a face.
        faces_.push_back(Face(t));
        Face &face = faces_.back();

        // Add edges.
        AddEdge_(face, v0, v1, edge_map);
        AddEdge_(face, v1, v2, edge_map);
        AddEdge_(face, v2, v0, edge_map);
    }
}

#if XXXX
PolyMesh::PolyMesh(const PolyMeshBuilder &builder) {
    // XXXX
}
#endif

PolyMesh::Edge & PolyMesh::NextEdgeInFace(const Edge &edge) {
    auto next_edge = [edge](const EdgeVec &edges) {
        const size_t index = edge.GetIndexInFace() + 1;
        return edges[index == edges.size() ? 0 : index];
    };

    const Face &face = edge.GetFace();
    const int hole_index = edge.GetFaceHoleIndex();
    if (hole_index >= 0) {
        ASSERT(static_cast<size_t>(hole_index) < face.GetHoleCount());
        return *next_edge(face.GetHoleEdges()[hole_index]);
    }
    else {
        return *next_edge(face.GetOuterEdges());
    }
}

PolyMesh::Edge & PolyMesh::PreviousEdgeInFace(const Edge &edge) {
    auto previous_edge = [edge](const EdgeVec &edges) {
        const size_t index = edge.GetIndexInFace();
        return edges[index > 0 ? index - 1 : edges.size() - 1];
    };

    const Face &face = edge.GetFace();
    const int hole_index = edge.GetFaceHoleIndex();
    if (hole_index >= 0) {
        ASSERT(static_cast<size_t>(hole_index) < face.GetHoleCount());
        return *previous_edge(face.GetHoleEdges()[hole_index]);
    }
    else {
        return *previous_edge(face.GetOuterEdges());
    }
}

PolyMesh::EdgeVec PolyMesh::GetVertexEdges(Edge &start_edge) {
    EdgeVec edges;
    Edge *edge = &start_edge;
    do {
        ASSERT(edge->GetOppositeEdge());
        edge = &NextEdgeInFace(*edge->GetOppositeEdge());
        ASSERT(&start_edge.GetV0() == &edge->GetV0());
        edges.push_back(edge);
    } while (edge != &start_edge);
    return edges;
}

TriMesh PolyMesh::ToTriMesh() const {
    TriMesh mesh;

    // Triangulate all faces and get a list of vertex indices.
    const std::vector<size_t> indices = GetTriangleIndices_(*this);

    // Make sure all resulting points are unique.
    Point3fMap pt_map(0);  // XXXX Maybe use precision for rounding.
    std::unordered_map<size_t, size_t> index_map;
    for (size_t i = 0; i < vertices_.size(); ++i)
        index_map[i] = pt_map.Add(vertices_[i].GetPoint());

    // The Point3fMap now contains all unique points, and the index_map maps
    // each original PolyMesh Vertex index to an index in the Point3fMap.
    mesh.points = pt_map.GetPoints();
    mesh.indices = Util::ConvertVector<int, size_t>(
        indices, [&index_map](const size_t &i){
            return static_cast<int>(index_map[i]); });
    return mesh;
}

void PolyMesh::AddEdge_(Face &face, Vertex &v0, Vertex &v1,
                        EdgeMap_ &edge_map, int hole_index) {
    // Add to the vector.
    edges_.push_back(Edge(edges_.size(), v0, v1, face, hole_index,
                          face.GetOuterEdges().size()));
    Edge &edge = edges_.back();

    // If the opposite edge already exists, connect the two.
    auto it = edge_map.find(EdgeHashKey_(v1, v0));
    if (it != edge_map.end())
        edge.ConnectOpposite_(*it->second);

    const std::string key = EdgeHashKey_(v0, v1);
    ASSERTM(! Util::MapContains(edge_map, key), "Duplicate edge key: " + key);
    edge_map[key] = &edge;

    // Add it to the face.
    face.AddEdge_(hole_index, edge);
}

std::string PolyMesh::EdgeHashKey_(const Vertex &v0, const Vertex &v1) {
    return v0.ID() + "_" + v1.ID();
}
