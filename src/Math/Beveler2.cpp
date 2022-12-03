#include "Math/Beveler2.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <ion/math/vectorutils.h>

#include "Math/Bevel.h"
#include "Math/Linear.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Skeleton3D.h"
#include "Util/Assert.h"
#include "Util/General.h"

#include "Math/ToString.h" // XXXX
#include "Debug/Dump3dv.h" // XXXX

// XXXX
template <typename T>
static std::string PT(const T &t) { return Math::ToString(t, .01f); }

namespace {

// ----------------------------------------------------------------------------
// Helper types.
// ----------------------------------------------------------------------------

/// This determines how the points of a Profile are applied to an edge of the
/// original PolyMesh. "Forward" and "Reverse" are assigned to make the bevel
/// profiles consistent as possible across faces.
enum class Direction_ {
    kUnknown,
    kForward,
    kReverse,
};

static Direction_ GetOppositeDirection_(Direction_ dir) {
    ASSERT(dir != Direction_::kUnknown);
    return dir == Direction_::kForward ?
        Direction_::kReverse : Direction_::kForward;
}

/// An EdgeProfile_ stores information about how a Profile is applied at the
/// starting vertex (v0) of an edge of the original PolyMesh. The points of the
/// EdgeProfile_ are added as vertices in a PolyMeshBuilder and the resulting
/// indices are stored here.  Profile vertices are always ordered in a
/// counterclockwise direction around the v0 vertex.
struct EdgeProfile_ {
    Direction_          direction = Direction_::kUnknown;
    std::vector<GIndex> indices;
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
/// The \c points_per_side value will be the same as P for the outermost ring.
/// Each Ring_ created inside another one has 2 fewer points per side.
struct Ring_ {
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

// ----------------------------------------------------------------------------
// Beveler2_ class. XXXX
// ----------------------------------------------------------------------------

class Beveler2_ {
  public:
    /// Applies the given Bevel to the given PolyMesh.
    Beveler2_(const PolyMesh &mesh, const Bevel &bevel);

    /// Returns the resulting beveled PolyMesh.
    PolyMesh GetResultPolyMesh();

  private:
    // Some shorthand.
    using EdgeVec_  = std::vector<PolyMesh::Edge *>;
    using IndexVec_ = std::vector<GIndex>;
    using RingVec_  = std::vector<Ring_>;

    /// Original PolyMesh to bevel.
    const PolyMesh                                           &mesh_;

    /// Bevel to apply.
    const Bevel                                              &bevel_;

    /// Straight 3D skeleton for the original PolyMesh.
    Skeleton3D                                               skeleton_;

    /// Maps a PolyMesh::Vertex pointer to a skeleton vertex index.
    std::unordered_map<const PolyMesh::Vertex *, size_t>     skel_vertex_map_;

    // PolyMeshBuilder used to construct the beveled PolyMesh.
    PolyMeshBuilder                                          pmb_;

    /// Maps each PolyMesh::Edge pointer to the PolyMeshBuilder index of the
    /// new offset vertex created for it.
    std::unordered_map<const PolyMesh::Edge *, size_t>       edge_vertex_map_;

    /// Maps each PolyMesh::Edge pointer to the EdgeProfile_ created for it.
    std::unordered_map<const PolyMesh::Edge *, EdgeProfile_> edge_profile_map_;


    /// XXXX
    void InitSkeleton_();

    /// Returns the index of the skeleton vertex at the other end of the
    /// bisector edge from the starting vertex of the given PolyMesh::Edge
    /// within its face. Asserts if it is not found.
    size_t GetSkeletonVertex_(const PolyMesh::Edge &edge);

    /// XXXX
    void AddOffsetFaces_();

    /// XXXX
    void CreateEdgeProfiles_();

    /// Assigns a direction to an edge of the original PolyMesh and its
    /// opposite edge, moving on to neighbors as well.
    void AssignEdgeDirection_(const PolyMesh::Edge &edge, Direction_ dir);

    /// XXXX
    void SetEdgeProfileIndices_(const PolyMesh::Edge &edge);

    /// Adds faces joining all profile points across each original edge of the
    /// PolyMesh.
    void AddEdgeFaces_();

    /// Adds faces joining all profile points around each original vertex of
    /// the PolyMesh.
    void AddVertexFaces_();

    // XXXX
    void AddVertexFaces_(const EdgeVec_ &edges, const IndexVec_ &indices);

    // XXXX
    void AddInnerVertexFaces_(const IndexVec_ &indices,
                              size_t points_per_side);

    /// XXXX
    Beveler2_::RingVec_ BuildRings_(size_t edge_count,
                                    const IndexVec_ &outer_indices);

    /// XXXX
    void PositionRingPoints_(const EdgeVec_ &edges, RingVec_ &rings);

    /// XXXX
    void AddInterRingFaces_(const RingVec_ &rings);

    /// Applies the bevel profile between the two given endpoints in a plane
    /// perpendicular to the given Edge. Returns the positions of the resulting
    /// interior points.
    std::vector<Point3f> ApplyProfileBetweenPoints_(const PolyMesh::Edge &edge,
                                                    const Point3f &p0,
                                                    const Point3f &p1);

    /// XXXX
    void DumpSkeleton_(const PolyMesh &mesh, const Skeleton3D &skeleton);
};

Beveler2_::Beveler2_(const PolyMesh &mesh, const Bevel &bevel) :
    mesh_(mesh),
    bevel_(bevel) {

    // Set up the Skeleton3D and map for its vertices.
    InitSkeleton_();

    // Using the skeleton, create offset vertices for each face of the PolyMesh
    // and add the vertices to the PolyMeshBuilder. Store the resulting indices
    // in a the edge_vertex_map_ and create faces joining them.
    AddOffsetFaces_();

    // Set up EdgeProfile_ instances for all PolyMesh edges and store them in
    // the edge_profile_map_.
    CreateEdgeProfiles_();

    // Add faces joining profile points across edges and around vertices.
    AddEdgeFaces_();
    AddVertexFaces_();
}

PolyMesh Beveler2_::GetResultPolyMesh() {
    const PolyMesh result_mesh = pmb_.BuildPolyMesh();

    {  // XXXX
        const bool add_orig_mesh = false;

        Debug::Dump3dv dump("/tmp/RMESH.3dv", "Result Beveler2");
        dump.SetLabelFontSize(40 - 4 * bevel_.profile.GetPointCount());
        Debug::Dump3dv::LabelFlags label_flags;
        label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
        dump.SetLabelFlags(label_flags);
        dump.AddPolyMesh(result_mesh);
        if (add_orig_mesh) {
            dump.SetExtraPrefix("M_");
            label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
            dump.SetLabelFlags(label_flags);
            dump.AddPolyMesh(mesh_);
        }
    }

    return result_mesh;
}

void Beveler2_::InitSkeleton_() {
    // Build the 3D straight skeleton for the PolyMesh.
    skeleton_.BuildForPolyMesh(mesh_);
    DumpSkeleton_(mesh_, skeleton_);  // XXXX

    // Map PolyMesh vertices to skeleton vertices.
    const auto &skel_vertices = skeleton_.GetVertices();
    for (size_t i = 0; i < skel_vertices.size(); ++i) {
        const int index = skel_vertices[i].source_index;
        // Add only skeleton vertices that correspond to PolyMesh vertices.
        if (index >= 0) {
            ASSERT(static_cast<size_t>(index) < mesh_.vertices.size());
            const auto &pmv = mesh_.vertices[index];
            ASSERT(! Util::MapContains(skel_vertex_map_, pmv));
            skel_vertex_map_[pmv] = i;
        }
    }
    ASSERT(skel_vertex_map_.size() == mesh_.vertices.size());
}

size_t Beveler2_::GetSkeletonVertex_(const PolyMesh::Edge &edge) {
    // Get the starting vertex for the Edge and the previous and next vertices
    // in the same Face.
    const auto &vstart = edge.v0;
    const auto &vnext  = edge.v1;
    const auto &vprev  = edge.PreviousEdgeInFace().v0;
    const size_t skel_vstart = skel_vertex_map_.at(vstart);
    const size_t skel_vnext  = skel_vertex_map_.at(vnext);
    const size_t skel_vprev  = skel_vertex_map_.at(vprev);

    // Find the skeleton bisector for the starting vertex of the face.
    // XXXX Speed this up?
    const int vni = static_cast<int>(skel_vnext);
    const int vpi = static_cast<int>(skel_vprev);
    for (const auto &edge: skeleton_.GetEdges()) {
        if ((edge.bisected_index0 == vni && edge.bisected_index1 == vpi) ||
            (edge.bisected_index1 == vni && edge.bisected_index0 == vpi)) {
            if (edge.v0_index == skel_vstart)
                return edge.v1_index;
            else if (edge.v1_index == skel_vstart)
                return edge.v0_index;
        }
    }
    ASSERTM(false, "No bisector found for " + vstart->ToString());
    return 0;
}

void Beveler2_::AddOffsetFaces_() {
    using ion::math::AngleBetween;
    using ion::math::Normalized;
    using ion::math::Sine;

    auto add_border = [&](const PolyMesh::EdgeVec &edges){
        PolyMesh::IndexVec indices;
        for (const auto &edge: edges) {
            const size_t svi = GetSkeletonVertex_(*edge);

            // Get the original vertex and the corresponding skeleton vertex.
            const Point3f &p0 = edge->v0->point;
            const Point3f &p1 = skeleton_.GetVertices()[svi].point;

            // Use trigonometry to find the position P along the skeleton
            // bisector edge that is S units from either of the original edges,
            // where S is the bevel scale.
            const Vector3f edge_vec     = edge->GetUnitVector();
            const Vector3f bisector_vec = Normalized(p1 - p0);
            const Anglef   angle        = AngleBetween(edge_vec, bisector_vec);
            const float    dist         = bevel_.scale / Sine(angle);
            const Point3f  pos          = p0 + dist * bisector_vec;

            // Add the new vertex to the offset face.
            const size_t index = pmb_.AddVertex(pos);
            indices.push_back(index);

            // Store the correspondence from the original edge to the new
            // vertex.
            edge_vertex_map_[edge] = index;
        }
        return indices;
    };

    for (const auto &face: mesh_.faces) {
        pmb_.AddPolygon(add_border(face->outer_edges));
        for (auto &hole: face->hole_edges)
            pmb_.AddHole(add_border(hole));
    }
}

void Beveler2_::CreateEdgeProfiles_() {
    // Create a default EdgeProfile_ instance for each PolyMesh edge.
    for (const auto &edge: mesh_.edges)
        edge_profile_map_[edge] = EdgeProfile_();

    // Assign directions to the EdgeProfile_ instances.
    for (auto &edge: mesh_.edges)
        AssignEdgeDirection_(*edge, Direction_::kForward);

    // Set up the indices in the EdgeProfile_ for each PolyMesh edge, adding
    // interior profile points to the PolyMeshBuilder if necessary.
    for (auto &edge: mesh_.edges)
        SetEdgeProfileIndices_(*edge);
}

void Beveler2_::AssignEdgeDirection_(const PolyMesh::Edge &edge,
                                     Direction_ dir) {
    // Do nothing if already assigned.
    EdgeProfile_ &ep = edge_profile_map_.at(&edge);
    if (ep.direction == Direction_::kUnknown) {
        EdgeProfile_    &opp_ep  = edge_profile_map_.at(edge.opposite_edge);
        const Direction_ opp_dir = GetOppositeDirection_(dir);
        ep.direction     = dir;
        opp_ep.direction = opp_dir;

        // Go to neighbor edges within the face and try to apply a consistent
        // direction.
        AssignEdgeDirection_(edge.NextEdgeInFace(), dir);
        AssignEdgeDirection_(edge.opposite_edge->NextEdgeInFace(), opp_dir);
    }
}

void Beveler2_::SetEdgeProfileIndices_(const PolyMesh::Edge &edge) {
    const size_t i0 = edge_vertex_map_.at(&edge.PreviousEdgeAroundVertex());
    const size_t i1 = edge_vertex_map_.at(&edge);

    const size_t count = bevel_.profile.GetPointCount();

    EdgeProfile_ &ep = edge_profile_map_.at(&edge);
    ep.indices.reserve(count);

    // Starting vertex.
    ep.indices.push_back(i0);

    // Interior vertices, if any. Reverse if necessary to maintain proper
    // direction.
    if (count > 2U) {
        const Point3f p0 = pmb_.GetVertex(i0);
        const Point3f p1 = pmb_.GetVertex(i1);

        std::vector<Point3f> pts;
        if (ep.direction == Direction_::kReverse) {
            pts = ApplyProfileBetweenPoints_(edge, p1, p0);
            std::reverse(pts.begin(), pts.end());
        }
        else {
            pts = ApplyProfileBetweenPoints_(edge, p0, p1);
        }

        for (size_t i = 0; i < pts.size(); ++i)
            ep.indices.push_back(pmb_.AddVertex(pts[i]));
    }

    // Ending vertex.
    ep.indices.push_back(i1);
}

std::vector<Point3f> Beveler2_::ApplyProfileBetweenPoints_(
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

void Beveler2_::AddEdgeFaces_() {
    // This set is used to keep opposite edges from being processed.
    std::unordered_set<const PolyMesh::Edge *> processed_edges;
    for (const auto &edge: mesh_.edges) {
        if (! Util::MapContains(processed_edges, edge)) {
            processed_edges.insert(edge->opposite_edge);

            const auto &ei0 = edge_profile_map_.at(edge).indices;
            const auto &ei1 = edge_profile_map_.at(edge->opposite_edge).indices;

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

void Beveler2_::AddVertexFaces_() {
    // Use a set to determine which vertices have been processed, since
    // multiple edges begin at any PolyMesh vertex.
    std::unordered_set<const PolyMesh::Vertex *> processed_vertices;

    for (const auto &edge: mesh_.edges) {
        if (! Util::MapContains(processed_vertices, edge->v0)) {
            processed_vertices.insert(edge->v0);

            // Get all edges around the vertex in counterclockwise order.
            const auto vertex_edges = PolyMesh::GetVertexEdges(*edge);

            // Collect indices of all PolyMeshBuilder vertices around the
            // vertex. Since the last vertex of each profile is the same as the
            // first vertex of the previous one, skip the last vertex for each.
            IndexVec_ indices;
            for (const auto &vertex_edge: vertex_edges) {
                const auto &ep = edge_profile_map_.at(vertex_edge);
                Util::AppendVector(ep.indices, indices);
                indices.pop_back();
            }
            if (edge->v0->id == "V7")
                std::cerr << "XXXX INDICES: "
                          << Util::JoinItems(indices) << "\n";

            AddVertexFaces_(vertex_edges, indices);
        }
    }
}

void Beveler2_::AddVertexFaces_(const EdgeVec_ &edges,
                                const IndexVec_ &indices) {
    // If NE is the number of edges and NP is the number of profile points,
    // then the number of points in a ring of vertices around the vertex is:
    //    NE * (NP - 1).
    const size_t profile_size = bevel_.profile.GetPointCount();
    ASSERT(indices.size() == edges.size() * (profile_size - 1));

    // 0 or 1 interior points are handled specially.
    if (profile_size == 2U || profile_size == 3U) {
        AddInnerVertexFaces_(indices, profile_size);
    }

    // General case: compute points on inner rings and process them.
    else {
#if XXXX
        RingVec_ rings = BuildRings_(edges.size(), indices);
        PositionRingPoints_(edges, rings);
        AddInterRingFaces_(rings);

        // Handle innermost ring.
        const Ring_ &inner = rings.back();
        AddInnerVertexFaces_(inner.indices, inner.points_per_side);
#endif
    }
}

void Beveler2_::AddInnerVertexFaces_(const IndexVec_ &indices,
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
#if XXXX

            const GIndex i0 = indices[(i - 1) % index_count];
            const GIndex i1 = indices[i];
            const GIndex i2 = indices[(i + 1) % index_count];
            inner_indices.push_back(i1);
            std::cerr << "XXXX Tri " << i0 << " " << i1 << " " << i2 << "\n";
            pmb_.AddTriangle(i0, i1, i2);
#endif
        }
        //std::cerr << "XXXX Inner = " << Util::JoinItems(inner_indices) << "\n";
        //pmb_.AddPolygon(inner_indices);
    }
}

#if XXXX
Beveler2_::RingVec_ Beveler2_::BuildRings_(size_t edge_count,
                                           const IndexVec_ &outer_indices) {
    // Compute the number of rings that are needed. Each ring has 2 fewer
    // points per edge than the ring surrounding it.
    const size_t profile_size = bevel_.profile.GetPointCount();
    const size_t ring_count   = profile_size / 2;
    RingVec_ rings(ring_count);

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

void Beveler2_::PositionRingPoints_(const EdgeVec_ &edges, RingVec_ &rings) {
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

            // Apply the profile to get the interior points.
            const auto pts = ApplyProfileBetweenPoints_(*edges[side], p0, p1);

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

void Beveler2_::AddInterRingFaces_(const RingVec_ &rings) {
    // Starting from the outside (first ring), connect the vertices between
    // each pair of rings.
    // XXXX N quads per edge, where N = NP for inner ring.

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

void Beveler2_::DumpSkeleton_(const PolyMesh &mesh,
                              const Skeleton3D &skeleton) {
    Debug::Dump3dv dump("/tmp/BMESH.3dv", "Beveler2");
    dump.SetLabelFontSize(60);
    dump.SetExtraPrefix("M_");
    dump.AddPolyMesh(mesh);
    dump.SetExtraPrefix("S_");
    dump.SetLabelOffset(Vector3f(1, 1, .5f));
    dump.AddSkeleton3D(skeleton);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Beveler2 functions.
// ----------------------------------------------------------------------------

TriMesh Beveler2::ApplyBevel(const TriMesh &mesh, const Bevel &bevel) {
    // Convert the TriMesh to a PolyMesh and merge coplanar faces.
    PolyMesh poly_mesh(mesh);
    MergeCoplanarFaces(poly_mesh);

    // Use a Beveler2_ to do the rest.
    Beveler2_ beveler(poly_mesh, bevel);
    return beveler.GetResultPolyMesh().ToTriMesh();
}
