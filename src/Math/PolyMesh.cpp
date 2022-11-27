#include "Math/PolyMesh.h"

#include <cmath>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <ion/math/vectorutils.h>

#include "Math/Triangulation.h"
#include "Math/Linear.h"
#include "Math/Point3fMap.h"
#include "Math/Polygon.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"

namespace {

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

// Shorthand.
typedef PolyMesh::Edge      Edge;
typedef PolyMesh::EdgeVec   EdgeVec;
typedef PolyMesh::Face      Face;
typedef PolyMesh::FaceVec   FaceVec;
typedef PolyMesh::Vertex    Vertex;
typedef PolyMesh::VertexVec VertexVec;
typedef PolyMesh::IndexVec  IndexVec;

/// Maps an Edge ID to an Edge instance.
typedef std::unordered_map<std::string, Edge *> EdgeMap_;

/// Returns a unique hash key for an edge between two vertices.
static std::string EdgeHashKey_(const Vertex &v0, const Vertex &v1) {
    return v0.id + "_" + v1.id;
}

/// Converts a vector of Edge pointers to a vector of points using the V0
/// vertex of each edge.
static std::vector<Point3f> EdgesToPoints_(const EdgeVec &edges) {
    return Util::ConvertVector<Point3f, Edge *>(
        edges, [](const Edge *e){ return e->v0->point; });
}

/// Converts a vector of Edge pointers to a vector of Vertex pointers using the
/// V0 vertex of each edge.
static VertexVec EdgesToVertices_(const EdgeVec &edges) {
    return Util::ConvertVector<Vertex *, Edge *>(
        edges, [](const Edge *e){ return e->v0; });
}

/// Implements PolyMesh::GetFaceVertices().
static void GetFaceVertices_(const Face &face, VertexVec &vertices,
                             std::vector<size_t> &border_counts) {
    vertices.clear();
    border_counts.clear();

    // Adds vertices from a border to vertices and the size to border_counts.
    auto add_border_verts = [&vertices, &border_counts](const EdgeVec &edges){
        Util::AppendVector(EdgesToVertices_(edges), vertices);
        border_counts.push_back(edges.size());
    };

    add_border_verts(face.outer_edges);
    for (const auto &hole: face.hole_edges)
        add_border_verts(hole);
}

/// Sets up a new edge, adding it to its face and connecting it to its opposite
/// edge. The EdgeMap_ is used to access the opposite edge.
static void SetUpEdge_(Edge &edge, EdgeMap_ &edge_map) {
    // If the opposite edge already exists, connect the two.
    auto it = edge_map.find(EdgeHashKey_(*edge.v1, *edge.v0));
    if (it != edge_map.end()) {
        ASSERT(it->second);
        Edge &opp = *it->second;
        ASSERT(! edge.opposite_edge);
        ASSERT(! opp.opposite_edge);
        edge.opposite_edge = &opp;
        opp.opposite_edge  = &edge;
    }

    // Add the new edge to the map.
    const std::string key = EdgeHashKey_(*edge.v0, *edge.v1);
    ASSERTM(! Util::MapContains(edge_map, key), "Duplicate edge key: " + key);
    edge_map[key] = &edge;

    // Add it to the proper border in its face.
    ASSERT(edge.face);
    Face &face = *edge.face;
    EdgeVec &edges = edge.face_hole_index >= 0 ?
        face.hole_edges[edge.face_hole_index] : face.outer_edges;
    edge.index_in_face = edges.size();
    edges.push_back(&edge);
}

/// Projects points from vertices forming a face onto one of the Cartesian
/// planes, using the given normal to help choose a plane. Returns the
/// projected 2D points.
static std::vector<Point2f> ProjectPoints_(const VertexVec &vertices,
                                           const Vector3f &normal) {

    // Figure out which of the 3 Cartesian planes is best for projection by
    // using the largest component of the normal.
    const int dim = GetMaxAbsElementIndex(normal);

    // Get the two dimensions for projecting.
    const int dim0 = (dim + 1) % 3;
    const int dim1 = (dim + 2) % 3;

    // Project all the vertices onto that plane. If projecting along the
    // negative normal, negate one of the dimensions to create a clockwise
    // ordering.
    const bool negate = normal[dim] < 0;
    auto func = [dim0, dim1, negate](const Vertex *v){
        return Point2f(v->point[dim0],
                       negate ? -v->point[dim1] : v->point[dim1]);
    };
    return Util::ConvertVector<Point2f, Vertex *>(vertices, func);
}

/// Returns a vector of Vertex pointers representing triangles for one Face of
/// a PolyMesh.
static VertexVec TriangulateFace_(const Face &face) {
    using ion::math::LengthSquared;

    // Vertex pointers of resulting triangles.
    VertexVec tri_verts;

    // Convenience.
    const auto get_v = [&](int index){ return face.outer_edges[index]->v0; };

    // Common triangulation case: no holes, 3 edges.
    const bool has_holes = ! face.hole_edges.empty();
    if (! has_holes && face.outer_edges.size() == 3U) {
        tri_verts.assign({ get_v(0), get_v(1), get_v(2) });
    }
    // Another common triangulation case: no holes, 4 edges. Split into two
    // triangles by the shorter diagonal - this should work for all quads, even
    // concave ones.
    else if (! has_holes && face.outer_edges.size() == 4U) {
        const auto &v0 = get_v(0);
        const auto &v1 = get_v(1);
        const auto &v2 = get_v(2);
        const auto &v3 = get_v(3);
        if (LengthSquared(v2->point - v0->point) <=
            LengthSquared(v3->point - v1->point))
            tri_verts.assign({ v0, v1, v2,  v0, v2, v3 });
        else
            tri_verts.assign({ v1, v2, v3,  v3, v0, v1 });
    }
    // General case: either there are holes or outer boundary has > 4 vertices.
    // Project all of the points into a 2D polygon based on the largest normal
    // component. Then triangulate using CGAL.
    else {
        VertexVec           vertices;
        std::vector<size_t> border_counts;
        GetFaceVertices_(face, vertices, border_counts);
        const std::vector<Point2f> points2D =
            ProjectPoints_(vertices, face.GetNormal());
        const auto tri_indices =
            TriangulatePolygon(Polygon(points2D, border_counts));
        // Convert the triangulated indices to Vertex pointers.
        tri_verts = Util::ConvertVector<Vertex *, GIndex>(
            tri_indices, [&vertices](const GIndex &i){ return vertices[i]; });
    }

    return tri_verts;
}

/// Returns a list of triangle indices representing all faces of a PolyMesh
// after triangulation.
static IndexVec GetTriangleIndices_(const PolyMesh &poly_mesh) {
    // This maps a Vertex pointer to its index in the full list of vertices for
    // the PolyMesh.
    std::unordered_map<Vertex *, GIndex> vertex_map;
    for (size_t i = 0; i < poly_mesh.vertices.size(); ++i)
        vertex_map[poly_mesh.vertices[i]] = i;

    // Triangulate the vertices of each face, getting indices for each Vertex
    // triangle. Map these to original Vertex indices.
    IndexVec indices;
    for (auto &face: poly_mesh.faces) {
        // Skip faces with zero area.
        if (ComputeArea(EdgesToPoints_(face->outer_edges)) > .001f) {
            const VertexVec tri_verts = TriangulateFace_(*face);
            IndexVec tri_indices = Util::ConvertVector<GIndex, Vertex *>(
                tri_verts,
                [&vertex_map](Vertex *v){ return vertex_map.find(v)->second; });
            Util::AppendVector(tri_indices, indices);
        }
    }
    return indices;
}

}   // anonymous namespace

// ----------------------------------------------------------------------------
// PolyMesh::Feature class functions.
// ----------------------------------------------------------------------------

PolyMesh::Feature::Feature(const std::string &prefix, int index) :
    id(prefix + Util::ToString(index)) {
}

// ----------------------------------------------------------------------------
// PolyMesh::Vertex class functions.
// ----------------------------------------------------------------------------

std::string PolyMesh::Vertex::ToString() const {
    return id + ": " + Util::ToString(point);
}

// ----------------------------------------------------------------------------
// PolyMesh::Edge class functions.
// ----------------------------------------------------------------------------

PolyMesh::Edge::Edge(int id, Vertex &v0_in, Vertex &v1_in, Face &face_in,
                     int face_hole_index_in) :
    Feature("E", id), v0(&v0_in), v1(&v1_in), face(&face_in),
    face_hole_index(face_hole_index_in), index_in_face(-1),
    opposite_edge(nullptr) {
    // The index_in_face is set when the Edge is added to its Face.
}

Vector3f PolyMesh::Edge::GetUnitVector() const {
    return ion::math::Normalized(v1->point - v0->point);
}

Edge & PolyMesh::Edge::NextEdgeInFace() const {
    ASSERT(face_hole_index < 0 ||
           static_cast<size_t>(face_hole_index) < face->GetHoleCount());
    EdgeVec &edges = face_hole_index >= 0 ?
        face->hole_edges[face_hole_index] : face->outer_edges;
    const size_t index = index_in_face + 1;
    return *edges[index == edges.size() ? 0 : index];
}

Edge & PolyMesh::Edge::PreviousEdgeInFace() const {
    ASSERT(face_hole_index < 0 ||
           static_cast<size_t>(face_hole_index) < face->GetHoleCount());
    EdgeVec &edges = face_hole_index >= 0 ?
        face->hole_edges[face_hole_index] : face->outer_edges;
    return *edges[index_in_face > 0 ? index_in_face - 1 : edges.size() - 1];
}

Edge & PolyMesh::Edge::NextEdgeAroundVertex() const {
    ASSERT(opposite_edge);
    Edge &next_edge = opposite_edge->NextEdgeInFace();
    ASSERT(next_edge.v0 == v0);
    return next_edge;
}

std::string PolyMesh::Edge::ToString() const {
    return id + " from " + v0->id + " to " + v1->id +
        ", face " + face->id +
        ", opp " + (opposite_edge ? opposite_edge->id : "NULL");
}

// ----------------------------------------------------------------------------
// PolyMesh::Face class functions.
// ----------------------------------------------------------------------------

const Vector3f & PolyMesh::Face::GetNormal() const {
    if (normal_ == Vector3f::Zero() && outer_edges.size() >= 3U)
        normal_ = ComputeNormal(EdgesToPoints_(outer_edges));
    return normal_;
}

void PolyMesh::Face::ReplaceEdge(Edge &old_edge, Edge &new_edge) {
    auto it = std::find(outer_edges.begin(), outer_edges.end(), &old_edge);
    ASSERT(it != outer_edges.end());

    // If the face already contains the new edge, just remove the old
    // one. Otherwise, replace it.
    if (std::find(outer_edges.begin(), outer_edges.end(),
                  &new_edge) != outer_edges.end())
        outer_edges.erase(it);
    else
        *it = &new_edge;

    // Reindex all remaining edges in the face.
    ReindexEdges();
}

void PolyMesh::Face::ReindexEdges() {
    auto reindex = [](EdgeVec &edges, int hole_index){
        for (size_t i = 0; i < edges.size(); ++i) {
            edges[i]->face_hole_index = hole_index;
            edges[i]->index_in_face   = i;
        }
    };

    reindex(outer_edges, -1);
    for (size_t h = 0; h < hole_edges.size(); ++h)
        reindex(hole_edges[h], h);
}

std::string PolyMesh::Face::ToString(bool on_one_line) const {
    auto edge_string = [&](const EdgeVec &ev){
        std::string s = "[";
        for (auto &e: ev)
            s+= " " + e->id;
        s += " ] [";
        for (auto &e: ev)
            s += " " + e->v0->id;
        s += " ]";
        if (! on_one_line)
            s += "\n";
        return s;
    };

    std::string fs = id + ": " + edge_string(outer_edges);
    if (! hole_edges.empty()) {
        fs += " { HOLES:";
        for (size_t i = 0; i < hole_edges.size(); ++i)
            fs += Util::ToString(i) + ":" + edge_string(hole_edges[i]);
        fs += "}";
        if (! on_one_line)
            fs += "\n";
    }

    return fs;
}

// ----------------------------------------------------------------------------
// PolyMesh class functions.
// ----------------------------------------------------------------------------


PolyMesh::PolyMesh(const TriMesh &mesh) {
    vertices.reserve(mesh.points.size());
    for (size_t i = 0; i < mesh.points.size(); ++i)
        vertices.push_back(new Vertex(i, mesh.points[i]));

    // Map used to find opposite edges.
    EdgeMap_ edge_map;

    // Build faces and edges.
    const size_t tri_count = mesh.GetTriangleCount();
    for (size_t t = 0; t < tri_count; ++t) {
        Vertex &v0 = *vertices[mesh.indices[3 * t + 0]];
        Vertex &v1 = *vertices[mesh.indices[3 * t + 1]];
        Vertex &v2 = *vertices[mesh.indices[3 * t + 2]];

        // Add a face.
        Face *face = new Face(t);
        faces.push_back(face);

        Edge *new_edges[3];
        new_edges[0] = new Edge(edges.size() + 0, v0, v1, *face, -1);
        new_edges[1] = new Edge(edges.size() + 1, v1, v2, *face, -1);
        new_edges[2] = new Edge(edges.size() + 2, v2, v0, *face, -1);

        for (int i = 0; i < 3; ++i) {
            Edge *e = new_edges[i];
            SetUpEdge_(*e, edge_map);
            edges.push_back(e);
        }
    }
}

PolyMesh::PolyMesh(const std::vector<Point3f> &points,
                   const std::vector<Border> &borders) {
    // Add vertices.
    for (size_t i = 0; i < points.size(); ++i)
        vertices.push_back(new Vertex(i, points[i]));

    // Map used to find opposite edges.
    EdgeMap_ edge_map;

    // Adds edges for a border.
    const auto add_edges = [this, &edge_map](const IndexVec &indices,
                                             Face &face, int hole_index){
        for (size_t i = 0; i < indices.size(); ++i) {
            Vertex *v0 = vertices[indices[i]];
            Vertex *v1 = vertices[indices[(i + 1) % indices.size()]];
            // Do not add degenerate edges.
            if (v0 != v1) {
                Edge *e = new Edge(edges.size(), *v0, *v1, face, hole_index);
                SetUpEdge_(*e, edge_map);
                edges.push_back(e);
            }
        }
    };

    bool cur_face_valid = false;
    for (const auto &border: borders) {
        if (! border.is_hole) {
            // Outer border of face. Start a new face.
            Face *face = new Face(faces.size());
            add_edges(border.indices, *face, -1);

            // Do not add degenerate faces.
            cur_face_valid = face->outer_edges.size() >= 3U &&
                face->GetNormal() != Vector3f::Zero();
            if (cur_face_valid)
                faces.push_back(face);
            else
                delete face;
        }
        // Otherwise, this is a hole in the current face. If the face is valid,
        // add the hole.
        else if (cur_face_valid) {
            ASSERT(! faces.empty());
            Face *face = faces.back();
            int hole_index = face->hole_edges.size();
            face->hole_edges.resize(face->hole_edges.size() + 1);
            add_edges(border.indices, *face, hole_index);
        }
    }
}

PolyMesh::~PolyMesh() {
    for (auto &e: edges)
        delete e;
    for (auto &f: faces)
        delete f;
    for (auto &v: vertices)
        delete v;
}

void PolyMesh::GetFaceVertices(const Face &face, VertexVec &vertices,
                               std::vector<size_t> &border_counts) {
    GetFaceVertices_(face, vertices, border_counts);
}

PolyMesh::EdgeVec PolyMesh::GetVertexEdges(Edge &start_edge) {
    EdgeVec edges;
    Edge *edge = &start_edge;
    do {
        ASSERT(edge->opposite_edge);
        edge = &edge->NextEdgeAroundVertex();
        ASSERT(start_edge.v0 == edge->v0);
        edges.push_back(edge);
    } while (edge != &start_edge);
    return edges;
}

TriMesh PolyMesh::ToTriMesh() const {
    TriMesh mesh;

    // Triangulate all faces and get a list of vertex indices.
    const std::vector<GIndex> indices = GetTriangleIndices_(*this);

    // Make sure all resulting points are unique.
    Point3fMap pt_map(0);  /// \todo Maybe use precision for rounding.
    std::unordered_map<GIndex, GIndex> index_map;
    for (size_t i = 0; i < vertices.size(); ++i)
        index_map[i] = pt_map.Add(vertices[i]->point);

    // The Point3fMap now contains all unique points, and the index_map maps
    // each original PolyMesh Vertex index to an index in the Point3fMap.
    mesh.points = pt_map.GetPoints();
    mesh.indices = Util::ConvertVector<GIndex, GIndex>(
        indices, [&index_map](const GIndex &i){ return index_map[i]; });
    return mesh;
}

void PolyMesh::Dump(const std::string &when) const {
    std::cout << "=== " << when << ": PolyMesh with "
              << vertices.size() << " vertices:\n";
    for (auto &v: vertices)
        std::cout << "  " << v->ToString() << "\n";

    std::cout << "=== ... and " << faces.size() << " faces:\n";
    for (auto &f: faces)
        std::cout << "  " << f->ToString(false);

    std::cout << "=== ... and " << edges.size() << " edges:\n";
    for (auto &e: edges)
        std::cout << "  " << e->ToString() << "\n";
}
