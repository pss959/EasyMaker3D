#include "Math/PolyMesh.h"

#include <cmath>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

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
typedef PolyMesh::Edge      Edge;
typedef PolyMesh::EdgeVec   EdgeVec;
typedef PolyMesh::Face      Face;
typedef PolyMesh::FaceVec   FaceVec;
typedef PolyMesh::Vertex    Vertex;
typedef PolyMesh::VertexVec VertexVec;
typedef std::vector<size_t> IndexVec;

//! Maps an Edge ID to an Edge instance.
typedef std::unordered_map<std::string, Edge *> EdgeMap_;

//! Returns a unique hash key for an edge between two vertices.
static std::string EdgeHashKey_(const Vertex &v0, const Vertex &v1) {
    return v0.id + "_" + v1.id;
}

//! Converts a vector of Edge pointers to a vector of points using the V0
//! vertex of each edge.
static std::vector<Point3f> EdgesToPoints_(const EdgeVec &edges) {
    return Util::ConvertVector<Point3f, Edge *>(
        edges, [](const Edge *e){ return e->v0->point; });
}

//! Converts a vector of Edge pointers to a vector of Vertex pointers using the
//! V0 vertex of each edge.
static VertexVec EdgesToVertices_(const EdgeVec &edges) {
    return Util::ConvertVector<Vertex *, Edge *>(
        edges, [](const Edge *e){ return e->v0; });
}

//! Projects points from vertices forming a face onto one of the Cartesian
//! planes, using the given normal to help choose a plane. Returns the
//! projected 2D points.
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

//! Returns a vector of Vertex pointers representing triangles for one Face of
//! a PolyMesh.
static VertexVec TriangulateFace_(const Face &face) {
    using ion::math::LengthSquared;

    // Collect vertices from all Face borders into one list. Keep track of
    // the size of each border added.
    VertexVec vertices;
    IndexVec  border_counts;

    // Adds vertices from one border to vertices and the size to border_counts.
    auto add_border_verts = [&vertices, &border_counts](const EdgeVec &edges){
        Util::AppendVector(EdgesToVertices_(edges), vertices);
        border_counts.push_back(edges.size());
    };

    add_border_verts(face.outer_edges);
    for (const auto &hole: face.hole_edges)
        add_border_verts(hole);

    // Vertex indices of resulting triangles.
    IndexVec tri_indices;

    // Common triangulation case: no holes, 3 vertices.
    if (border_counts.size() == 1U && vertices.size() == 3U) {
        tri_indices.assign({ 0, 1, 2 });
    }
    // Another common triangulation case: no holes, 4 vertices. Split into two
    // triangles by the shorter diagonal - this should work for all quads, even
    // concave ones.
    else if (border_counts.size() == 1U && vertices.size() == 4U) {
        if (LengthSquared(vertices[2]->point - vertices[0]->point) <=
            LengthSquared(vertices[3]->point - vertices[1]->point))
            tri_indices.assign({ 0, 1, 2,  0, 2, 3 });
        else
            tri_indices.assign({ 1, 2, 3,  3, 0, 1 });
    }
    // General case: either there are holes or outer boundary has > 4 vertices.
    // Project all of the points into 2D based on the largest normal
    // component. Then triangulate using CGAL.
    else {
        const std::vector<Point2f> points2D =
            ProjectPoints_(vertices, face.GetNormal());
        tri_indices = TriangulatePolygon(Polygon(points2D, border_counts));
    }

    // Convert the indices to Vertex pointers.
    return Util::ConvertVector<Vertex *, size_t>(
        tri_indices, [&vertices](const size_t i){ return vertices[i]; });
}

//! Returns a list of triangle indices representing all faces of a PolyMesh
// after triangulation.
static IndexVec GetTriangleIndices_(const PolyMesh &poly_mesh) {
    // This maps a Vertex pointer to its index in the full list of vertices for
    // the PolyMesh.
    std::unordered_map<Vertex *, size_t> vertex_map;
    for (size_t i = 0; i < poly_mesh.vertices.size(); ++i)
        vertex_map[poly_mesh.vertices[i]] = i;

    // Triangulate the vertices of each face, getting indices for each Vertex
    // triangle. Map these to original Vertex indices.
    IndexVec indices;
    for (auto &face: poly_mesh.faces) {
        // Skip faces with zero area.
        if (ComputeArea(EdgesToPoints_(face->outer_edges)) > .001f) {
            const VertexVec tri_verts = TriangulateFace_(*face);
            IndexVec tri_indices = Util::ConvertVector<size_t, Vertex *>(
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
    // std::cerr << "XXXX Created " << id << " @ " << this << "\n";
}

PolyMesh::Feature::~Feature() {
    // std::cerr << "XXXX Destroying " << id << " @ " << this << "\n";
}

// ----------------------------------------------------------------------------
// PolyMesh::Edge class functions.
// ----------------------------------------------------------------------------

PolyMesh::Edge::Edge(int id, Vertex &v0_in, Vertex &v1_in, Face &face_in,
                     int face_hole_index_in, int index_in_face_in) :
    Feature("E", id), v0(&v0_in), v1(&v1_in), face(&face_in),
    face_hole_index(face_hole_index_in), index_in_face(index_in_face_in),
    opposite_edge(nullptr) {
}

Vector3f PolyMesh::Edge::GetUnitVector() const {
    return ion::math::Normalized(v1->point - v0->point);
}

Edge * PolyMesh::Edge::NextEdgeInFace() const {
    ASSERT(face_hole_index < 0 ||
           static_cast<size_t>(face_hole_index) < face->GetHoleCount());
    EdgeVec &edges = face_hole_index >= 0 ?
        face->hole_edges[face_hole_index] : face->outer_edges;
    const size_t index = index_in_face + 1;
    return edges[index == edges.size() ? 0 : index];
}

Edge * PolyMesh::Edge::PreviousEdgeInFace() const {
    ASSERT(face_hole_index < 0 ||
           static_cast<size_t>(face_hole_index) < face->GetHoleCount());
    EdgeVec &edges = face_hole_index >= 0 ?
        face->hole_edges[face_hole_index] : face->outer_edges;
    return edges[index_in_face > 0 ? index_in_face - 1 : edges.size() - 1];
}

void PolyMesh::Edge::ConnectOpposite_(Edge &opposite) {
    // This should happen at most once.
    ASSERT(! opposite_edge);
    ASSERT(! opposite.opposite_edge);
    opposite_edge          = &opposite;
    opposite.opposite_edge = this;
}

// ----------------------------------------------------------------------------
// PolyMesh::Face class functions.
// ----------------------------------------------------------------------------

const Vector3f & PolyMesh::Face::GetNormal() const {
    if (normal_ == Vector3f::Zero() && outer_edges.size() >= 3U)
        normal_ = ComputeNormal(EdgesToPoints_(outer_edges));
    return normal_;
}

void PolyMesh::Face::AddEdge_(int hole_index, Edge &edge) {
    if (hole_index < 0) {
        outer_edges.push_back(&edge);
    }
    else {
        ASSERT(static_cast<size_t>(hole_index) < hole_edges.size());
        hole_edges[hole_index].push_back(&edge);
    }
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
        new_edges[0] = new Edge(edges.size() + 0, v0, v1, *face, -1, 0);
        new_edges[1] = new Edge(edges.size() + 1, v1, v2, *face, -1, 1);
        new_edges[2] = new Edge(edges.size() + 2, v2, v0, *face, -1, 2);

        for (int i = 0; i < 3; ++i) {
            Edge *e = new_edges[i];
            edges.push_back(e);

            // If the opposite edge already exists, connect the two.
            auto it = edge_map.find(EdgeHashKey_(*e->v1, *e->v0));
            if (it != edge_map.end()) {
                ASSERT(it->second);
                e->ConnectOpposite_(*it->second);
            }

            // Add to the map.
            const std::string key = EdgeHashKey_(*e->v0, *e->v1);
            ASSERTM(! Util::MapContains(edge_map, key),
                    "Duplicate edge key: " + key);
            edge_map[key] = e;

            // Add it to the face.
            face->outer_edges.push_back(e);
        }
    }
}

#if XXXX
PolyMesh::PolyMesh(const PolyMeshBuilder &builder) {
    // XXXX
}
#endif

PolyMesh::~PolyMesh() {
    for (auto &e: edges)
        delete e;
    for (auto &f: faces)
        delete f;
    for (auto &v: vertices)
        delete v;
}

PolyMesh::EdgeVec PolyMesh::GetVertexEdges(Edge &start_edge) {
    EdgeVec edges;
    Edge *edge = &start_edge;
    do {
        ASSERT(edge->opposite_edge);
        edge = edge->opposite_edge->NextEdgeInFace();
        ASSERT(start_edge.v0 == edge->v0);
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
    for (size_t i = 0; i < vertices.size(); ++i)
        index_map[i] = pt_map.Add(vertices[i]->point);

    // The Point3fMap now contains all unique points, and the index_map maps
    // each original PolyMesh Vertex index to an index in the Point3fMap.
    mesh.points = pt_map.GetPoints();
    mesh.indices = Util::ConvertVector<int, size_t>(
        indices, [&index_map](const size_t &i){
            return static_cast<int>(index_map[i]); });
    return mesh;
}

void PolyMesh::Dump(const std::string &when) const {
    auto dump_edges = [](const std::string &prefix, const EdgeVec &ev){
        for (auto &e: ev)
            std::cout << prefix << " " << e->id;
        std::cout << " ] [";
        for (auto &e: ev)
            std::cout << " " << e->v0->id;
        std::cout << " ]\n";
    };

    std::cout << "=== " << when << ": PolyMesh with "
              << vertices.size() << " vertices:\n";
    for (auto &v: vertices)
        std::cout << "  " << v->id << ": " << v->point << "\n";

    std::cout << "=== ... and " << faces.size() << " faces:\n";
    for (auto &f: faces) {
        std::cout << "  " << f->id << ": [";
        dump_edges("", f->outer_edges);
        if (! f->hole_edges.empty()) {
            std::cout << " { HOLES:";
            for (size_t i = 0; i < f->hole_edges.size(); ++i)
                dump_edges(Util::ToString(i) + ":", f->hole_edges[i]);
            std::cout << "}\n";
        }
    }

    std::cout << "=== ... and " << edges.size() << " edges:\n";
    for (auto &e: edges) {
        std::cout << "  " << e->id << " from " << e->v0->id
                  << " to " << e->v1->id
                  << ", face " << e->face->id
                  << ", opp " << (e->opposite_edge ?
                                  e->opposite_edge->id : "NULL") << "\n";
    }
}
