#include "Math/Beveler.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <ion/math/vectorutils.h>

#include "Math/Bevel.h"
#include "Math/Linear.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Util/Assert.h"
#include "Util/General.h"

#include "Debug/Dump3dv.h" // XXXX
#include "Math/ToString.h" // XXXX

template <typename T> // XXXX
static std::string PT(const T &t) { return Math::ToString(t, .01f); }

static bool XXXX_v(const PolyMesh::Vertex &vertex) {
    //return vertex.id == "V7" || vertex.id == "V2";
    // return vertex.id == "V12";
    //return true;
    return false;
}

#if XXXX
static bool XXXX_e(const PolyMesh::Edge &edge) {
    // return edge.id == "E52" || edge.id == "E49";
    return false;
}
#endif

namespace {

// ----------------------------------------------------------------------------
// Beveler_ class. This does all of the work of applying a bevel to a PolyMesh.
// ----------------------------------------------------------------------------

class Beveler_ {
  public:
    /// Applies the given Bevel to the given PolyMesh.
    Beveler_(const PolyMesh &mesh, const Bevel &bevel, PolyMesh &result_mesh);

  private:
    /// An EdgeData_ instance stores information about an edge of the original
    /// PolyMesh.
    struct EdgeData_ {
        /// Used to maintain a more consistent ordering of the profile
        /// direction for the entire PolyMesh.
        bool is_reversed = false;

        /// Set to true if this edge is beveled, based on the angle formed by
        /// its adjacent faces and the #max_angle field of the Bevel.
        bool is_beveled = false;

        /// Stores the indices of the PolyMeshBuilder vertices created to apply
        /// the profile to the edge at its #v0 vertex, ordered counterclockwise
        /// around the vertex. If the bevel profile has N points, there will be
        /// exactly N indices stored per edge. (This is done for convenience,
        /// even though the last point is always the same as the first point of
        /// the next edge around the vertex).
        std::vector<GIndex> indices;
    };

    struct Ring_;

    // Some shorthand.
    using EdgeVec_     = std::vector<PolyMesh::Edge *>;
    using IndexVec_    = std::vector<GIndex>;
    using RingVec_     = std::vector<Ring_>;
    using VertexEdgeMap_ = std::unordered_map<const PolyMesh::Vertex *,
                                              PolyMesh::Edge *>;
    using EdgeDataMap_ = std::unordered_map<const PolyMesh::Edge *, EdgeData_>;

    /// Original PolyMesh to bevel.
    const PolyMesh  &mesh_;

    /// Bevel to apply.
    const Bevel     &bevel_;

    /// PolyMeshBuilder used to construct the beveled PolyMesh.
    PolyMeshBuilder pmb_;

    /// Stores an edge starting at each vertex of the original PolyMesh.
    VertexEdgeMap_  vertex_edge_map_;

    /// Maps each original PolyMesh::Edge pointer to the EdgeData_ instance
    /// created for it.
    EdgeDataMap_    edge_data_map_;

    /// Creates and initializes an EdgeData_ instance for the given edge of the
    /// original PolyMesh and its opposite, storing them in the
    /// #edge_data_map_. Possibly recurses on neighbor edges as well.
    void InitEdgeData_(const PolyMesh::Edge &edge, bool is_reversed);

    /// XXXX
    void CreateOffsetVertices_(const PolyMesh::Vertex &vertex);

    /// XXXX
    GIndex GetOffsetPoint_(const PolyMesh::Edge &e0, const PolyMesh::Edge &e1);

    /// XXXX
    GIndex InterpolateOffsetPoints_(const PolyMesh::Edge &edge,
                                    const PolyMesh::Edge &prev,
                                    const PolyMesh::Edge &next);

    /// Computes and returns the position of the offset vertex for the vertex
    /// between two beveled edges.
    Point3f ComputeOffsetPosition_(const PolyMesh::Edge &e0,
                                   const PolyMesh::Edge &e1);

    /// XXXX
    void AddFaceBorder_(const EdgeVec_ &border_edges, bool is_hole);

    /// This ensures that the EdgeData_ for the given edge has the proper
    /// vertex indices. If the profile has interior points, it applies the
    /// profile curve to the offset endpoints of the given beveled edge and
    /// adds the resulting interior points.
    void FillEdgeIndices_(const PolyMesh::Edge &edge);

    /// Adds faces joining all profile points across each beveled edge of the
    /// original PolyMesh.
    void AddEdgeFaces_();

    /// Adds faces joining all profile points around each original vertex of
    /// the PolyMesh.
    void AddVertexFaces_();

    /// Adds faces joining all profile points for a single vertex. All edges
    /// starting at the vertex are supplied along with the point indices.
    void AddFacesForVertex_(const EdgeVec_ &edges, const IndexVec_ &indices);

    /// Adds faces for a vertex for the innermost ring with 2 or 3 points per
    /// side.
    void AddInnerVertexFaces_(const IndexVec_ &indices, size_t points_per_side);

    /// Builds and returns a vector of Ring_ instances representing the points
    /// around a vertex when the number of points per side is more than 3. This
    /// adds vertices to the PolyMeshBuilder but leaves their positions unset.
    RingVec_ BuildRings_(const EdgeVec_ &edges, const IndexVec_ &outer_indices);

    /// Sets the positions for the points corresponding to all Ring_ instances
    /// in the vector.
    void PositionRingPoints_(const EdgeVec_ &edges, RingVec_ &rings);

    /// Adds faces to the PolyMeshBuilder to fill the spaces between all pairs
    /// of Ring_ instances in the vector.
    void AddInterRingFaces_(const RingVec_ &rings);

    /// Applies the bevel profile between the two given endpoints in a plane
    /// perpendicular to the given Edge. Returns the positions of the resulting
    /// interior points.
    std::vector<Point3f> ApplyProfileBetweenPoints_(const PolyMesh::Edge &edge,
                                                    const Point3f &p0,
                                                    const Point3f &p1);

    /// Returns true if the given edge is beveled.
    bool IsEdgeBeveled_(const PolyMesh::Edge &edge) const {
        return edge_data_map_.at(&edge).is_beveled;
    }
};

/// A Ring_ is used to add faces around each vertex of the original PolyMesh
/// when the bevel Profile has 4 or more points. It stores the indices of some
/// number of PolyMeshBuilder vertices around the vertex.
///
/// Consider an original PolyMesh vertex that has E edges starting at it. Each
/// of these edges results in a bevel profile in a plane perpendicular to that
/// edge, offset from the vertex. These profiles are joined to form a ring.  A
/// Ring_ is divided into "sides", each of which corresponds to one of the
/// bevel profiles created for an edge.
///
/// If P is the number of points in the Profile and E is the number of edges,
/// the total number of points in the outermost ring will be E * (P - 1),
/// because the last vertex of each side of the ring is the same as the first
/// vertex of the next side.
///
/// The #points_per_side value will be the same as P for the outermost ring.
/// Each Ring_ created inside another one has 2 fewer points per side.
struct Beveler_::Ring_ {
    size_t              level;            ///< Ring level (0 == outside).
    size_t              side_count;       ///< Number of sides in the ring.
    size_t              points_per_side;  ///< Number of vertices per side.
    std::vector<GIndex> indices;          ///< PolyMeshBuilder vertex indices.

    /// This is used to keep track of how many time each point of the ring has
    /// been repositioned.
    std::vector<size_t> reposition_counts;

    /// Returns the index of the side before the given one.
    size_t GetPrevSide(size_t side) const {
        return (side + side_count - 1) % side_count;
    };

    /// Returns the index of the side after the given one.
    size_t GetNextSide(size_t side) const {
        return (side + 1) % side_count;
    };

    /// Returns the index within the Ring indices of the ith vertex on a side.
    size_t GetIndexOnSide(size_t side, size_t i) const {
        ASSERT(i < points_per_side);
        return side * (points_per_side - 1) + i;
    }

    /// Combines GetPrevSide() and GetIndexOnSide().
    size_t GetIndexOnPrevSide(size_t side, size_t i) const {
        return GetIndexOnSide(GetPrevSide(side), i);
    };

    /// Combines GetNextSide() and GetIndexOnSide().
    size_t GetIndexOnNextSide(size_t side, size_t i) const {
        return GetIndexOnSide(GetNextSide(side), i);
    };

    /// Returns the PolyMeshBuilder vertex index of the indexed ring vertex.
    GIndex GetVertexIndex(size_t index) const {
        return indices[index % indices.size()];
    }

    /// Converts to a string for debugging.
    std::string ToString() const {
        return std::string("Ring " + Util::ToString(level) + " PPS=" +
                           Util::ToString(points_per_side) + ": " +
                           Util::JoinItems(indices));
    }
};

Beveler_::Beveler_(const PolyMesh &mesh, const Bevel &bevel,
                   PolyMesh &result_mesh) : mesh_(mesh), bevel_(bevel) {
    /* XXXX

      - Every beveled original edge results in 2 new edges.
      - Every unbeveled original edge remains in the result, possibly with
         different endpoints (depending on neighbor edges).

      - Each end of a beveled edge has N vertices (profile size).
      - Each end of an unbeveled edge has 1 vertex.

     */

#if 1 // XXXX
    {  // XXXX
        Debug::Dump3dv dump("/tmp/OMESH.3dv", "Original Beveler");
        dump.SetLabelFontSize(20);
        Debug::Dump3dv::LabelFlags label_flags;
        label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
        label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
        label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
        dump.SetLabelFlags(label_flags);
        dump.AddPolyMesh(mesh_);
    }
#endif

    // Store the first edge that starts at each vertex.
    for (const auto edge: mesh_.edges) {
        if (! Util::MapContains(vertex_edge_map_, edge->v0))
            vertex_edge_map_[edge->v0] = edge;
    }

    // Create and initialize an EdgeData_ instance for each original PolyMesh
    // edge.
    for (const auto &edge: mesh_.edges)
        InitEdgeData_(*edge, false);

    // XXXX
    for (const auto &vert: mesh_.vertices)
        CreateOffsetVertices_(*vert);

    // Create faces joining the offset vertices for faces.
    for (const auto &face: mesh_.faces) {
        // std::cerr << "XXXX Face " << face->id << " =================\n";
        AddFaceBorder_(face->outer_edges, false);
        for (auto &hole: face->hole_edges)
            AddFaceBorder_(hole, true);
    }

    // XXXX COLLAPSE NOW!

    for (auto &edge: mesh_.edges)
        FillEdgeIndices_(*edge);

    // Add faces joining profile points across edges and around vertices.
    AddEdgeFaces_();
#if XXXX
    AddVertexFaces_();
#endif

    // Construct the result PolyMesh and make sure the resulting PolyMesh has
    // no duplicate features.
    pmb_.BuildPolyMesh(result_mesh);
#if 0 // XXXX
    std::cerr << "XXXX result_mesh has " << result_mesh.vertices.size()
              << " verts and " << result_mesh.faces.size() << " faces\n";
#endif
#if 1 // XXXX
    {  // XXXX
        const bool add_orig_mesh = false;

        Debug::Dump3dv dump("/tmp/UMESH.3dv", "Unmerged Beveler");
        dump.SetLabelFontSize(20);
        Debug::Dump3dv::LabelFlags label_flags;
        label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
        //label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
        label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
        dump.SetLabelFlags(label_flags);
        dump.AddPolyMesh(result_mesh);
        if (add_orig_mesh) {
            dump.SetExtraPrefix("M_");
            label_flags.SetAll(false);
            // label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
            label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
            // label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
            dump.SetLabelFlags(label_flags);
            dump.AddPolyMesh(mesh_);
        }
    }
#endif
    MergeDuplicateFeatures(result_mesh, result_mesh);
#if 0 // XXXX
    {  // XXXX
        const bool add_orig_mesh = false;

        Debug::Dump3dv dump("/tmp/RMESH.3dv", "Result Beveler");
        dump.SetLabelFontSize(20);
        Debug::Dump3dv::LabelFlags label_flags;
        label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
        //label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
        label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
        dump.SetLabelFlags(label_flags);
        dump.AddPolyMesh(result_mesh);
        if (add_orig_mesh) {
            dump.SetExtraPrefix("M_");
            label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
            label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
            dump.SetLabelFlags(label_flags);
            dump.AddPolyMesh(mesh_);
        }
    }
#endif
}

void Beveler_::InitEdgeData_(const PolyMesh::Edge &edge, bool is_reversed) {
    // Do nothing if already handled.
    if (Util::MapContains(edge_data_map_, &edge))
        return;

    // Create a new EdgeData_ for the edge and its opposite.
    EdgeData_ data;
    EdgeData_ opp_data;

    // Set the directions.
    data.is_reversed     = is_reversed;
    opp_data.is_reversed = ! is_reversed;

    // Compute the angle formed by the edge's faces and determine if the edge
    // should be beveled.
    const Vector3f &normal0 = edge.face->GetNormal();
    const Vector3f &normal1 = edge.opposite_edge->face->GetNormal();
    const auto angle =
        Anglef::FromDegrees(180) - ion::math::AngleBetween(normal0, normal1);
    data.is_beveled = opp_data.is_beveled = angle <= bevel_.max_angle;

    // Store the data in the map.
    edge_data_map_[&edge]              = data;
    edge_data_map_[edge.opposite_edge] = opp_data;

    // Unbeveled edges do not affect any others. For beveled edges, go to
    // neighbors and try to apply a consistent direction.
    if (data.is_beveled) {
        InitEdgeData_(edge.NextEdgeInFace(),                  is_reversed);
        InitEdgeData_(edge.opposite_edge->NextEdgeInFace(), ! is_reversed);
    }
}

void Beveler_::CreateOffsetVertices_(const PolyMesh::Vertex &vertex) {
    // Process all edges starting at this vertex in counterclockwise order.  In
    // this phase, treat all edges as beveled. For each edge, store the index
    // of the offset point between the previous edge and this edge in the
    // EdgeData_.
    const PolyMesh::Edge &start_edge = *vertex_edge_map_.at(&vertex);
    const PolyMesh::Edge *edge = &start_edge;
    do {
        auto &data = edge_data_map_.at(edge);
        ASSERT(data.indices.empty());
        const auto &prev_edge = edge->PreviousEdgeAroundVertex();
        data.indices.push_back(GetOffsetPoint_(prev_edge, *edge));
        if (XXXX_v(vertex))
            std::cerr << "XXXX Created V" << data.indices.back()
                      << " for " << edge->id
                      << " using prev " << prev_edge.id << "\n";
        edge = &edge->NextEdgeAroundVertex();
    } while (edge != &start_edge);
}

GIndex Beveler_::GetOffsetPoint_(const PolyMesh::Edge &e0,
                                 const PolyMesh::Edge &e1) {
    ASSERT(e0.v0 == e1.v0);

    // Determine if the edges are beveled. At least one must be.
    const bool is_e0_beveled = IsEdgeBeveled_(e0);
    const bool is_e1_beveled = IsEdgeBeveled_(e1);
    ASSERT(is_e0_beveled || is_e1_beveled);

    Point3f pos;
    if (is_e0_beveled && is_e1_beveled)
        pos = ComputeOffsetPosition_(e0, e1);
    else if (is_e0_beveled)
        pos = e0.v0->point + bevel_.scale * e1.GetUnitVector();
    else
        pos = e0.v0->point + bevel_.scale * e0.GetUnitVector();

    // Add the new vertex.
    return pmb_.AddVertex(pos);
}

#if 0 // XXXX
GIndex Beveler_::InterpolateOffsetPoints_(const PolyMesh::Edge &edge,
                                          const PolyMesh::Edge &prev,
                                          const PolyMesh::Edge &next) {
    // Get the neighbor offset points.
    const EdgeData_ &prev_data = edge_data_map_.at(&prev);
    const EdgeData_ &next_data = edge_data_map_.at(&next);
    ASSERT(! prev_data.indices.empty());
    ASSERT(! next_data.indices.empty());
    const Point3f &pp = pmb_.GetVertex(prev_data.indices.back());
    const Point3f &np = pmb_.GetVertex(next_data.indices.front());

    // Add an offset point at the closest point to the line joining the offset
    // points of those edges.
    Point3f c0, c1;
    GetClosestLinePoints(edge.v0->point, edge.GetUnitVector(),
                         pp, ion::math::Normalized(np - pp), c0, c1);
    return pmb_.AddVertex(c0);
}
#endif

Point3f Beveler_::ComputeOffsetPosition_(const PolyMesh::Edge &e0,
                                         const PolyMesh::Edge &e1) {
    ASSERT(e0.v0 == e1.v0);
    const Vector3f e0_vec = e0.GetUnitVector();
    const Vector3f e1_vec = e1.GetUnitVector();

    Vector3f bisector = ion::math::Normalized(e0_vec + e1_vec);

    // Determine if the angle formed by the two edges is > 180 degrees. The
    // cross product will face in the same direction as the face normal for an
    // angle > 180 (reversed if either edge is in a hole.
    const Vector3f cross       = ion::math::Cross(e0_vec, e1_vec);
    const float    dot         = ion::math::Dot(cross, e0.face->GetNormal());
    const bool     is_over_180 = dot < 0;

    // Negate the bisector if the angle is > 180.
    if (is_over_180)
        bisector = -bisector;

    // Move along the bisector the correct distance based on the bevel scale.
    const Anglef angle = ion::math::AngleBetween(e0_vec, bisector);
    return e0.v0->point + (bevel_.scale / ion::math::Sine(angle)) * bisector;
}

void Beveler_::AddFaceBorder_(const EdgeVec_ &border_edges, bool is_hole) {
    // The indices in an EdgeData_ are ordered to be counterclockwise around a
    // vertex, where the first index is between the previous edge and the
    // EdgeData_'s edge. This means that accessing the correct index for a
    // border edge requires using the opposite.
    const auto get_index = [&](const PolyMesh::Edge &e){
        return edge_data_map_.at(e.opposite_edge).indices.front();
    };

    IndexVec_ indices;
    for (const auto edge: border_edges)
        indices.push_back(get_index(*edge));

    if (indices.size() >= 3U) {
        if (is_hole)
            pmb_.AddHole(indices);
        else
            pmb_.AddPolygon(indices);
    }
}

void Beveler_::FillEdgeIndices_(const PolyMesh::Edge &edge) {
    // The index of the starting vertex should already be stored.
    EdgeData_ &data = edge_data_map_.at(&edge);
    ASSERT(data.indices.size() == 1U);
    const size_t i0 = data.indices.front();

    // Access the starting vertex for the next edge; this is the same as the
    // ending vertex for this edge.
    const PolyMesh::Edge &next_edge = edge.NextEdgeAroundVertex();
    const EdgeData_      &next_data = edge_data_map_.at(&next_edge);
    ASSERT(! next_data.indices.empty());
    const size_t i1 = next_data.indices.front();

    // If the profile has interior points (N > 2), apply the profile.
    const size_t profile_size = bevel_.profile.GetPointCount();
    if (profile_size > 2U) {
        data.indices.reserve(profile_size - 1);

        // Compute the interior profile points.
        const Point3f &p0 = pmb_.GetVertex(i0);
        const Point3f &p1 = pmb_.GetVertex(i1);
        std::vector<Point3f> pts;
        if (data.is_reversed) {
            pts = ApplyProfileBetweenPoints_(edge, p1, p0);
            std::reverse(pts.begin(), pts.end());
        }
        else {
            pts = ApplyProfileBetweenPoints_(edge, p0, p1);
        }

        // Add the interior vertices and store the indices.
        for (const auto &pt: pts)
            data.indices.push_back(pmb_.AddVertex(pt));
    }

    // Always add the ending index for convenience.
    data.indices.push_back(i1);
}

void Beveler_::AddEdgeFaces_() {
    // This set is used to keep opposite edges from being processed.
    std::unordered_set<const PolyMesh::Edge *> processed_edges;
    for (const auto &edge: mesh_.edges) {
        if (! Util::MapContains(processed_edges, edge) &&
            IsEdgeBeveled_(*edge)) {
            processed_edges.insert(edge->opposite_edge);

            const auto &ei0 = edge_data_map_.at(edge).indices;
            const auto &ei1 = edge_data_map_.at(edge->opposite_edge).indices;

            // Order of profile vertices is always counterclockwise, so have to
            // use the opposite edge indices in reverse order here to match.
            ASSERT(ei0.size() == ei1.size());
            for (size_t i0 = 1; i0 < ei0.size(); ++i0) {
                const size_t i1 = ei1.size() - i0;
                pmb_.AddQuad(ei0[i0], ei0[i0 - 1], ei1[i1], ei1[i1 - 1]);
            }
        }
    }
}

#if 0 // XXXX
void Beveler_::AddVertexFaces_() {
    for (const auto &vertex: mesh_.vertices) {
        // Collect all edges around the vertex in counterclockwise order and
        // also collect indices of all PolyMeshBuilder vertices around the
        // vertex.
        EdgeVec_  vertex_edges;
        IndexVec_ indices;
        PolyMesh::Edge *edge = vertex_edge_map_.at(vertex);
        const size_t profile_size = bevel_.profile.GetPointCount();
        do {
            vertex_edges.push_back(edge);
            const auto &data = edge_data_map_.at(edge);

            const size_t index_count = data.indices.size();
            if (index_count <= 2U) {
                const size_t target_size = profile_size - 1;
                if (index_count == 2U && target_size == 1U) {
                    indices.push_back(data.indices.front());
                }
                else {
                    // Add the 1 or 2 indices.
                    Util::AppendVector(data.indices, indices);
                    // There have to be the N-1 indices for each edge, so
                    // duplicate if necessary for unbeveled edges.
                    if (index_count < target_size)
                        Util::AppendVector(IndexVec_(target_size - index_count,
                                                     data.indices.back()),
                                           indices);
                }
            }
            else {
                ASSERT(index_count == profile_size);
                Util::AppendVector(data.indices, indices);
                // The last index should always refer to the same vertex as the
                // start of the next one, so remove the last one.
                indices.pop_back();
            }
            PolyMesh::Edge &next_edge = edge->NextEdgeAroundVertex();
            const auto &next_data = edge_data_map_.at(&next_edge);
            // XXXX Special case.
            if (! data.is_beveled && ! next_data.is_beveled) {
                const auto &opp_data =
                    edge_data_map_.at(next_edge.opposite_edge);
#if 1 // XXXX
                std::cerr << "XXXX ==== Special case for " << edge->id
                          << "/" << data.indices.back()
                          << " and " << next_edge.id
                          << "/" << next_data.indices.front()
                          << ": TRI " << next_data.indices.front()
                          << " " << data.indices.back()
                          << " " << opp_data.indices.front()
                          << "\n";
#endif
                pmb_.AddTriangle(next_data.indices.front(),
                                 data.indices.back(),
                                 opp_data.indices.front());
            }
            edge = &next_edge;
        } while (edge != vertex_edges.front());
        if (XXXX_v(*vertex))
            std::cerr << "XXXX " << vertex->id << " Indices = "
                      << Util::JoinItems(indices) << "\n";
        AddFacesForVertex_(vertex_edges, indices);
    }
}

void Beveler_::AddFacesForVertex_(const EdgeVec_ &edges,
                                  const IndexVec_ &indices) {
    // If NE is the number of edges and NP is the number of profile points,
    // then the number of points in a ring of vertices around the vertex is:
    //    NE * (NP - 1).
    const size_t profile_size = bevel_.profile.GetPointCount();
    ASSERT(indices.size() == edges.size() * (profile_size - 1));

    // General case: compute points on inner rings and process them.
    if (profile_size > 3U) {
        RingVec_ rings = BuildRings_(edges, indices);
        PositionRingPoints_(edges, rings);
        AddInterRingFaces_(rings);

        // Handle innermost ring.
        const Ring_ &inner = rings.back();
        AddInnerVertexFaces_(inner.indices, inner.points_per_side);
    }

    // 0 or 1 interior points don't require extra work.
    else {
        AddInnerVertexFaces_(indices, profile_size);
    }
}

void Beveler_::AddInnerVertexFaces_(const IndexVec_ &indices,
                                    size_t points_per_side) {
    if (points_per_side == 2U) {
        // Special case for single point per side.
        pmb_.AddPolygon(indices);
    }

    else {
        ASSERT(points_per_side == 3U);
        // Special case for only one interior point per side. Add center points
        // as a polygon and add triangles from vertices to them.
        IndexVec_ inner_indices;
        const size_t index_count = indices.size();
        for (size_t i = 1; i < index_count; i += 2) {
            const GIndex i0 = indices[i];
            const GIndex i1 = indices[(i + 1) % index_count];
            const GIndex i2 = indices[(i + 2) % index_count];
            inner_indices.push_back(i0);
            pmb_.AddTriangle(i0, i1, i2);
        }
        pmb_.AddPolygon(inner_indices);
    }
}

Beveler_::RingVec_ Beveler_::BuildRings_(const EdgeVec_ &edges,
                                         const IndexVec_ &outer_indices) {
    // Compute the number of rings that are needed. Each ring has 2 fewer
    // points per edge than the ring surrounding it.
    const size_t profile_size = bevel_.profile.GetPointCount();
    const size_t ring_count   = profile_size / 2;
    RingVec_ rings(ring_count);

    const size_t edge_count = edges.size();

    for (size_t r = 0; r < ring_count; ++r) {
        Ring_ &ring = rings[r];
        ring.level = r;
        ring.side_count = edge_count;
        ring.points_per_side = profile_size - 2 * r;

        if (r == 0) {
            ring.indices = outer_indices;
        }
        else {
            // Create vertices for all inner rings. Positions are initialized
            // to zero so they can be accumulated and averaged later.
            ring.indices.resize(edge_count * (ring.points_per_side - 1));
            for (size_t i = 0; i < ring.indices.size(); ++i)
                ring.indices[i] = pmb_.AddVertex(Point3f::Zero());
        }
    }
    return rings;
}

void Beveler_::PositionRingPoints_(const EdgeVec_ &edges, RingVec_ &rings) {
    // The vertices of the outer ring are in the correct places. For each inner
    // ring, reposition its points by applying all or part of the bevel profile
    // to the appropriate end points in the outermost ring (level 0).
    //
    // Some inner points will be repositioned once, and some twice. Keep track
    // so the resulting points can be averaged if necessary.
    for (size_t i = 0; i < rings.size(); ++i) {
        Ring_ &ring = rings[i];
        ring.reposition_counts.assign(ring.indices.size(), 0);
    }

    const size_t profile_size = bevel_.profile.GetPointCount();

    // Work towards the innermost ring.
    const Ring_  &outer      = rings[0];
    const size_t  outer_pps  = outer.points_per_side;
    for (size_t r = 1; r < rings.size(); ++r) {
        Ring_ &ring = rings[r];
        for (size_t side = 0; side < ring.side_count; ++side) {
            // Get the endpoints from the outermost ring for applying the
            // profile. The first endpoint is on the previous side.
            const size_t i0 = outer.GetIndexOnPrevSide(side, outer_pps - 1 - r);
            const size_t i1 = outer.GetIndexOnNextSide(side, r);
            const Point3f &p0 = pmb_.GetVertex(outer.GetVertexIndex(i0));
            const Point3f &p1 = pmb_.GetVertex(outer.GetVertexIndex(i1));

            std::vector<Point3f> pts;
            // If the endpoints are very close (due to unbeveled edges), just
            // create copies of the endpoints. Otherwise, apply the profile to
            // get the interior points.
            if (AreClose(p0, p1))
                pts.assign(profile_size - 2, p0);
            else
                pts = ApplyProfileBetweenPoints_(*edges[side], p0, p1);

            // Store the appropriate points in the inner ring. The pts vector
            // stores just the interior points.
            for (size_t i = 0; i < ring.points_per_side; ++i) {
                const Point3f &pos = pts[r - 1 + i];
                const size_t index = i == ring.points_per_side - 1 ?
                    ring.GetIndexOnNextSide(side, 0) :
                    ring.GetIndexOnSide(side, i);
                const size_t v_index = ring.GetVertexIndex(index);
                size_t &count = ring.reposition_counts[index];

                if (count == 0U) {
                    pmb_.MoveVertex(v_index, pos);
                }
                else {
                    ASSERT(count == 1U);
                    const Point3f &cur_pos = pmb_.GetVertex(v_index);
                    pmb_.MoveVertex(v_index, .5f * (cur_pos + pos));
                }
                ++count;
            }
        }
    }
}

void Beveler_::AddInterRingFaces_(const RingVec_ &rings) {
    // Starting from the outside (first ring), connect the vertices between
    // each pair of rings. This creates N quadrilaterals per edge, where N is
    // the number of points per side for the inner ring.
    for (size_t r = 0; r + 1 < rings.size(); ++r) {
        const Ring_ &outer = rings[r];
        const Ring_ &inner = rings[r + 1];
        for (size_t side = 0; side < outer.side_count; ++side) {
            for (size_t i = 0; i < inner.points_per_side; ++i) {
                const size_t i0 =
                    outer.GetVertexIndex(outer.GetIndexOnSide(side, i));
                const size_t i1 =
                    outer.GetVertexIndex(outer.GetIndexOnSide(side, i + 1));
                const size_t i2 =
                    inner.GetVertexIndex(inner.GetIndexOnSide(side, i));
                // Special case for starting corner.
                const size_t i3 = i == 0 ?
                    outer.GetVertexIndex(
                        outer.GetIndexOnPrevSide(side,
                                                 outer.points_per_side - 2)) :
                    inner.GetVertexIndex(inner.GetIndexOnSide(side, i - 1));
                pmb_.AddQuad(i0, i1, i2, i3);
            }
        }
    }
}
#endif

std::vector<Point3f> Beveler_::ApplyProfileBetweenPoints_(
    const PolyMesh::Edge &edge, const Point3f &p0, const Point3f &p1) {
    // Compute a base point on the edge for the profile. Use the closest point
    // on the edge to the line from p0 to p1.
    Point3f c0, c1;
    GetClosestLinePoints(edge.v0->point, edge.GetUnitVector(),
                         p0, ion::math::Normalized(p1 - p0), c0, c1);
    const Point3f base_point = c0;

    // Compute the vectors from the base point to p0 and p1.
    Vector3f vec0 = p0 - base_point;
    Vector3f vec1 = p1 - base_point;

    // Create all interior profile points.
    const auto &prof_pts = bevel_.profile.GetPoints();
    std::vector<Point3f> result_pts;
    result_pts.reserve(prof_pts.size());
    for (const auto &pp: prof_pts) {
        result_pts.push_back(base_point +
                             (1 - pp[0]) * vec0 +
                             (1 - pp[1]) * vec1);
    }

    return result_pts;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Beveler functions.
// ----------------------------------------------------------------------------

TriMesh Beveler::ApplyBevel(const TriMesh &mesh, const Bevel &bevel) {
    // Convert the TriMesh to a PolyMesh and merge coplanar faces into
    // polygons.
    PolyMesh poly_mesh(mesh);
    MergeCoplanarFaces(poly_mesh);

    // Use a Beveler_ to do the rest.
    PolyMesh result_mesh;
    Beveler_ beveler(poly_mesh, bevel, result_mesh);

    return result_mesh.ToTriMesh();
}
