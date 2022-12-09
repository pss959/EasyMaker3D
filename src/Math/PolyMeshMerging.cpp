#include "Math/PolyMeshMerging.h"

#include <cmath>
#include <unordered_map>
#include <unordered_set>

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/Point3fMap.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Util/Assert.h"
#include "Util/General.h"

namespace {

// ----------------------------------------------------------------------------
// Helper types and classes.
// ----------------------------------------------------------------------------

// Shorthand.
typedef PolyMesh::Edge      Edge;
typedef PolyMesh::EdgeVec   EdgeVec;
typedef PolyMesh::Face      Face;
typedef PolyMesh::Vertex    Vertex;

/// This struct is used to count the number of edges each PolyMesh Vertex is
// part of, and to store the first two edges. It helps merge the two edges
// around a vertex with exactly 2 edges (which must be collinear) and to remove
// vertices with fewer than 2 edges.
struct VertexInfo_ {
    int  edge_count = 0;        ///< Number of edges the Vertex is in.
    Edge *e0        = nullptr;  ///< First edge the Vertex is in.
    Edge *e1        = nullptr;  ///< Second edge the Vertex is in.
};

/// Maps a Vertex to its VertexInfo_.
typedef std::unordered_map<Vertex *, VertexInfo_> VertexMap_;

/// Helper class for finding holes in a merged Face of a PolyMesh that is known
/// to have holes and storing them properly in the Face.
class HoleFinder_ {
  public:
    /// The constructor is passed the Face to process.
    HoleFinder_(Face &face) : face_(face) {
        borders_.resize(1);  // Outer border always exists.
    }

    /// Moves all edges forming holes into the Face's hole_edges.
    void StoreHoleEdges();

  private:
    Face                 &face_;    ///< The face being processed.
    std::vector<EdgeVec> borders_;  ///< Edges of all borders.

    /// Adds an edge to the first border that is empty or whose last edge is
    /// connected to the given edge.
    void AddEdge_(Edge &e);

    /// Returns the size of a border, computed as the sum of the 3 dimension
    /// sizes. (Since one dimension possibly has size zero, cannot use the
    /// product.) This is used to determine which border is the outermost.
    float GetBorderSize_(const EdgeVec &border);
};


/// Helper class for detecting and merging duplicate vertices in a PolyMesh.
class VertexMerger_ {
  public:
    /// The constructor is passed a PolyMeshBuilder instance used to add new
    /// vertices.
    explicit VertexMerger_(PolyMeshBuilder &pmb) : pmb_(pmb) {}

    /// Returns a vector of unique indices for all vertices in one border of a
    /// PolyMesh::Face.
    std::vector<GIndex> GetBorderIndices(const EdgeVec &border);

  private:
    PolyMeshBuilder &pmb_;
    /// Used to find duplicate vertex locations within a reasonable tolerance.
    Point3fMap                                 point_map_{ .0001f };
    /// Maps a Point3fMap index to the PolyMeshBuilder vertex index created for
    /// the point.
    std::unordered_map<GIndex, GIndex>         index_map_;
    /// Maps each original vertex to the index of the unique PolyMeshBuilder
    /// vertex at the same location.
    std::unordered_map<const Vertex *, GIndex> vertex_map_;

    /// Returns the PolyMeshBuilder index for a vertex.
    GIndex GetVertexIndex_(const Vertex &vert);
};

std::vector<GIndex> VertexMerger_::GetBorderIndices(const EdgeVec &border) {
    std::vector<GIndex> indices;
    for (const auto edge: border) {
        const GIndex index = GetVertexIndex_(*edge->v0);
        // Don't add consecutive duplicate vertices.
        if (indices.empty() || index != indices.back())
            indices.push_back(index);
    }
    return indices;
}

GIndex VertexMerger_::GetVertexIndex_(const Vertex &vert) {
    // Add to the Point3fMap and see if the vertex is a duplicate.
    Point3f pos;
    GIndex index = point_map_.Add(vert.point, &pos);
    if (Util::MapContains(index_map_, index)) {
        // This is a duplicate. Associate the vertex with the index of the
        // unique vertex at the same location.
        vertex_map_[&vert] = index_map_.at(index);
    }
    else {
        // The vertex has a unique location (so far). Insert it.
        index = pmb_.AddVertex(pos);
        index_map_[index]  = index;
        vertex_map_[&vert] = index;
    }
    return index;
}

// ----------------------------------------------------------------------------
// HoleFinder_ class functions.
// ----------------------------------------------------------------------------

void HoleFinder_::StoreHoleEdges() {
    // Add all edges to the borders_ vector.
    for (auto &edge: face_.outer_edges)
        AddEdge_(*edge);
    for (auto &hole: face_.hole_edges)
        for (auto &edge: hole)
            AddEdge_(*edge);
    ASSERT(borders_.size() > 1U);  // Must be at least one hole.

    // Figure out which border is the outside. It should have the largest
    // bounds.
    auto cmp = [this](const EdgeVec &b0, const EdgeVec &b1){
        return GetBorderSize_(b0) < GetBorderSize_(b1);
    };
    auto it = std::max_element(borders_.begin(), borders_.end(), cmp);
    ASSERT(it != borders_.end());

    // Set the outer edges to the outside border. Remove it from the vector and
    // assign the rest as hole borders.
    face_.outer_edges = *it;
    borders_.erase(it);
    face_.hole_edges = borders_;

    // Reindex all edges in the face.
    face_.ReindexEdges();
}

void HoleFinder_::AddEdge_(Edge &e) {
    // See if the edge is connected to an existing border.
    auto it = std::find_if(borders_.begin(), borders_.end(),
                           [&e](const EdgeVec &b){
                           return b.empty() || b.back()->v1 == e.v0; });
    // If so, add to it.
    if (it != borders_.end()) {
        it->push_back(&e);
    }
    // Otherwise, start a new border.
    else {
        borders_.resize(borders_.size() + 1);
        borders_.back().push_back(&e);
    }
}

float HoleFinder_::GetBorderSize_(const EdgeVec &border) {
    ASSERT(border.size() >= 3U);

    Bounds bounds;
    for (auto &e: border)
        bounds.ExtendByPoint(e->v0->point);
    const auto &size = bounds.GetSize();
    return size[0] + size[1] + size[2];
}

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Returns true if the faces on opposite sides of the given edge are coplanar.
static bool AreFacesCoplanar_(const Edge &e) {
    ASSERT(e.opposite_edge);
    const Face &f0 = *e.face;
    const Face &f1 = *e.opposite_edge->face;

    // Since the faces share an edge, they are coplanar if their normals are
    // very close to identical (dot = 1) or opposite (dot = -1).
    return std::fabs(ion::math::Dot(f0.GetNormal(), f1.GetNormal())) > .9995f;
}

/// Inserts an edge into a face at an appropriate spot by finding an existing
/// edge it is connected to.
void InsertEdgeIntoFace_(Face &face, Edge &edge) {
    // Find an edge that this one connects to at either end.
    int index = -1;
    for (size_t i = 0; index < 0 && i < face.outer_edges.size(); ++i) {
        Edge *e = face.outer_edges[i];
        if (edge.v1 == e->v0)
            index = i;
        else if (e->v1 == edge.v0)
            index = i + 1;
    }
    // If no edge is found, this probably starts a hole, so add it at
    // the end.
    if (index < 0)
        face.outer_edges.push_back(&edge);
    else
        face.outer_edges.insert(face.outer_edges.begin() + index, &edge);
}

/// Merges the face opposite common_edge into its face. All removed edges are
/// added to the removed_edges set.
static void MergeFaces_(Edge &common_edge,
                        std::unordered_set<Edge *> &removed_edges) {
    // common_edge is part of to_face and separates it from from_face.
    ASSERT(common_edge.opposite_edge);
    Face &to_face = *common_edge.face;
    ASSERT(&to_face != common_edge.opposite_edge->face);

    // Remove common_edge from to_face and insert all of the edges from the
    // source face except for the opposite edge and any other edge that also
    // borders to_face; the latter can occur with complex concave polygons.
    const int index = common_edge.index_in_face;
    ASSERT(&common_edge == to_face.outer_edges[index]);
    to_face.outer_edges.erase(to_face.outer_edges.begin() + index);
    Edge *from_edge = common_edge.opposite_edge;
    while (true) {
        from_edge = &from_edge->NextEdgeInFace();
        if (from_edge == common_edge.opposite_edge)
            break;

        // Add the edge to to_face. Find the correct spot, which is not always
        // the same as where the removed common_edge was.
        InsertEdgeIntoFace_(to_face, *from_edge);
    }

    // Make sure all edges in to_face refer to it. This has to be done after
    // the above loop so that NextEdgeInFace() refers to fromFace.
    for (auto &edge: to_face.outer_edges)
        edge->face = &to_face;

    // Add removed edges to the set.
    removed_edges.insert(&common_edge);
    removed_edges.insert(common_edge.opposite_edge);

    /// Vestigial edges can sometimes be left behind after merging more
    // than one face into the same face. Find any in the face and remove
    // them and their opposite edges.
    EdgeVec vestigial_edges;
    std::copy_if(to_face.outer_edges.begin(), to_face.outer_edges.end(),
                 std::back_inserter(vestigial_edges),
                 [](const Edge *e){ return e->IsVestigial(); });
    for (auto &ve: vestigial_edges) {
        removed_edges.insert(ve);
        removed_edges.insert(ve->opposite_edge);
    }
    Util::EraseIf(to_face.outer_edges,
                  [](const Edge *e){ return e->IsVestigial(); });

    // Reindex all remaining edges in the face.
    to_face.ReindexEdges();
}

/// Builds and returns a VertexMap_ for all vertices of the PolyMesh.
static VertexMap_ BuildVertexMap_(const PolyMesh &poly_mesh) {
    VertexMap_ vmap;

    // Create a default VertexInfo_ instance for each vertex.
    for (auto &v: poly_mesh.vertices)
        vmap[v] = VertexInfo_();

    // Count and store edges.
    for (auto &e: poly_mesh.edges) {
        ASSERT(Util::MapContains(vmap, e->v0));
        VertexInfo_ &info = vmap[e->v0];

        // Update the VertexInfo_ for the V0 vertex of the edge.
        if (info.edge_count == 0)
            info.e0 = e;
        else if (info.edge_count == 1)
            info.e1 = e;
        ++info.edge_count;
    }
    return vmap;
}

/// Merges two collinear edges. The from_edge is merged into to_edge and
/// from_edge is removed.
static void MergeEdges_(Edge &from_edge, Edge &to_edge) {
    // Edges must have the first vertex in common.
    ASSERT(from_edge.v0 == to_edge.v0);
    Vertex *v0 = from_edge.v1;  // New vertex at the start of to_edge

    // Edges are arranged like this:
    //
    //              from.opp         to
    //            ----------->  ------------>
    //         v0             vm          v1
    //            <-----------  <------------
    //                from         to.opp
    //
    // vm = common vertex to remove.

    // Extend to_edge's starting vertex and its opposite's ending vertex.
    to_edge.v0                = v0;
    to_edge.opposite_edge->v1 = v0;

    // Replace from_edge and its opposite in their faces.
    Edge &oppo_edge = *from_edge.opposite_edge;
    from_edge.face->ReplaceEdge(from_edge, *to_edge.opposite_edge);
    oppo_edge.face->ReplaceEdge(oppo_edge, to_edge);
}

/// Using the VertexMap_, this looks for vertices that are part of exactly 2
/// edges. Any such vertex must lie between two collinear edges. This removes
/// all such vertices from the PolyMesh and merges one collinear edge into the
/// other.
static void MergeCollinearEdges_(PolyMesh &poly_mesh, VertexMap_ &vmap) {
    // Maps an Edge to the Edge it was merged into, just in case another
    // Edge tries to merge with the one that is now gone.
    std::unordered_map<Edge *, Edge *> merged_edges;

    // Look for vertices that are part of exactly 2 edges.
    for (auto &v: poly_mesh.vertices) {
        ASSERT(Util::MapContains(vmap, v));
        const VertexInfo_ &vinfo = vmap[v];
        if (vinfo.edge_count == 2) {
            Edge *from_edge = vinfo.e0;
            Edge *to_edge   = vinfo.e1;

            // Don't merge from or to an Edge that is no longer around.
            while (Util::MapContains(merged_edges, from_edge))
                from_edge = merged_edges[from_edge];
            while (Util::MapContains(merged_edges, to_edge))
                to_edge   = merged_edges[to_edge];

            // Choose edges so that they have v0 in common.
            MergeEdges_(*from_edge, from_edge->v0 == to_edge->v0 ?
                        *to_edge : *to_edge->opposite_edge);

            merged_edges[from_edge]                = to_edge;
            merged_edges[from_edge->opposite_edge] = to_edge->opposite_edge;
        }
    }

    // Remove and delete merged edges.
    Util::EraseIf(poly_mesh.edges, [&merged_edges](Edge *e){
        return Util::MapContains(merged_edges, e); });
    for (auto it: merged_edges)
        delete it.first;
}

/// Returns true if a Face contains holes after merging.
static bool FaceHasHoles_(Face &f) {
    // If any edge is not connected to the previous one, there must be a
    // hole. And if there is a hole, there must be at least 2 disconnections.
    for (size_t i = 1; i < f.outer_edges.size(); ++i)
        if (f.outer_edges[i - 1]->v1 != f.outer_edges[i]->v0)
            return true;
    return false;
}

}   // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

void MergeCoplanarFaces(PolyMesh &poly_mesh) {
    // Set of removed edges.
    std::unordered_set<Edge *> removed_edges;

    // Map from a merged face to the face it was merged into.
    std::unordered_map<Face *, Face *> merged_faces;

    // Finds the real face to use for the given one following merges.
    auto get_unmerged_face = [&merged_faces](Face &f){
        auto it = merged_faces.find(&f);
        return it != merged_faces.end() ? it->second : &f;
    };

    // Find and remove all edges that are between coplanar faces, merging their
    // faces.
    for (auto &e: poly_mesh.edges) {
        ASSERTM(e->opposite_edge, e->id);

        // Skip edges already removed by their opposite edge or edges bounding
        // faces that were already merged.
        if (Util::MapContains(removed_edges, e) ||
            Util::MapContains(merged_faces, e->opposite_edge->face))
            continue;

        if (AreFacesCoplanar_(*e)) {
            // Get the face to merge into, following chains of previous merges.
            Face *to_face = get_unmerged_face(*e->face);
            ASSERT(to_face);

            // Store removed items before they are modified in MergeFaces_().
            Face *from_face = e->opposite_edge->face;
            merged_faces[from_face] = to_face;

            // Merge the faces and mark the removed face.
            MergeFaces_(*e, removed_edges);
            from_face->is_merged = true;
        }
    }

    // Remove all of the merged faces and edges from the PolyMesh.
    Util::EraseIf(poly_mesh.faces, [&merged_faces](Face *f){
        return Util::MapContains(merged_faces, f); });
    Util::EraseIf(poly_mesh.edges, [&removed_edges](Edge *e){
        return Util::MapContains(removed_edges, e); });

    // Also delete them.
    for (auto it: merged_faces)
        delete it.first;
    for (auto it: removed_edges)
        delete it;

    // Create a VertexInfo_ struct for each Vertex of the PolyMesh.
    VertexMap_ vmap = BuildVertexMap_(poly_mesh);

    // Get rid of any vertices adjoining only two collinear edges and merge
    // those edges.
    MergeCollinearEdges_(poly_mesh, vmap);

    // Remove and delete any vertices with fewer than 3 remaining edges.
    Util::EraseIf(poly_mesh.vertices, [&vmap](Vertex *v){
        ASSERT(Util::MapContains(vmap, v));
        return vmap[v].edge_count < 3;
    });
    for (auto it: vmap)
        if (it.second.edge_count < 3)
            delete it.first;

    // Process any faces that have holes to make sure their edges are stored
    // properly.
    for (auto &f: poly_mesh.faces)
        if (FaceHasHoles_(*f))
            HoleFinder_(*f).StoreHoleEdges();
}

void MergeDuplicateFeatures(const PolyMesh &poly_mesh, PolyMesh &result_mesh) {
    // Use a VertexMerger_ to remove duplicate vertices.
    PolyMeshBuilder pmb;
    VertexMerger_ vertex_merger(pmb);

    // Process each face with large enough area in the PolyMesh.
    std::vector<GIndex> indices;
    for (const auto face: poly_mesh.faces) {
        // Minimum area for a face to be added.
      if (face->GetOuterArea() > 0) {
            indices = vertex_merger.GetBorderIndices(face->outer_edges);
            if (indices.size() >= 3U) {
                pmb.AddPolygon(indices);

                // Check for holes.
                for (auto &hole: face->hole_edges) {
                    indices = vertex_merger.GetBorderIndices(hole);
                    if (indices.size() >= 3U)
                        pmb.AddHole(indices);
                }
            }
        }
    }

    pmb.BuildPolyMesh(result_mesh);
}
