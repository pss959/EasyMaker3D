#include "Math/Beveler2.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "Math/Bevel.h"
#include "Math/Linear.h"  // XXXX
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Skeleton3D.h"
#include "Util/Assert.h"
#include "Util/General.h"

#include "Debug/Dump3dv.h" // XXXX
#include "Util/String.h" // XXXX

namespace {

// ----------------------------------------------------------------------------
// Helper types.
// ----------------------------------------------------------------------------

/// This determines how the points of a Profile are applied at the first vertex
/// of an edge of the original PolyMesh. The orientation is relative when
/// looking along direction of the edge from the outside of the PolyMesh.
enum class Direction_ {
    kUnknown,
    kLeftToRight,
    kRightToLeft,
};

static Direction_ GetOppositeDirection_(Direction_ dir) {
    ASSERT(dir != Direction_::kUnknown);
    return dir == Direction_::kLeftToRight ?
        Direction_::kRightToLeft : Direction_::kLeftToRight;
}

/// An EdgeProfile_ stores information about how a Profile is applied to an
/// edge of the original PolyMesh. The points of the EdgeProfile_ are added as
/// vertices in a PolyMeshBuilder and the resulting indices are stored in here.
struct EdgeProfile_ {
    Direction_ direction = Direction_::kUnknown;

    /// Index of the first added vertex of the Profile.
    size_t end0_index = 0;
    /// Index of the last added vertex of the Profile.
    size_t end1_index = 0;

    /// Index of the first added interior vertex of the Profile. This is unused
    /// if the Profile is a simple bevel with only 2 points.
    size_t interior_index = 0;
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
    size_t              points_per_side;  ///< Number of vertices per side.
    std::vector<GIndex> indices;          ///< PolyMeshBuilder vertex indices.

    /// This is used to keep track of how many time each point of the ring has
    /// been repositioned.
    std::vector<size_t> reposition_counts;

    /// Returns a PolyMeshBuilder vertex index with vector range checking.
    GIndex GetIndex(int i) const {
        return indices[i < 0 ? indices.size() + i : (i + 1) % indices.size()];
    }

    /// Returns the index within the Ring indices of the ith vertex on a side.
    size_t GetSideIndex(size_t side, size_t i) const {
        ASSERT(i < points_per_side);
        return side * (points_per_side - 1) + i;
    }

    /// Returns the PolyMeshBuilder vertex index of the indexed ring vertex.
    GIndex GetVertexIndex(size_t index) const {
        ASSERT(index < indices.size());
        return indices[index];
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

    /// Adds faces joining all profile points around each original vertex of
    /// the PolyMesh.
    void AddVertexFaces_();

    // XXXX
    void AddVertexFaces_(const EdgeVec_ &edges, const IndexVec_ &indices);

    /// Adds faces joining all profile points across each original edge of the
    /// PolyMesh.
    void AddEdgeFaces_();

    /// XXXX
    Beveler2_::RingVec_ BuildRings_(size_t edge_count,
                                    const IndexVec_ &outer_indices);

    /// XXXX
    void PositionRingPoints_(const EdgeVec_ &edges, RingVec_ &rings);

    /// Applies the bevel profile between the two given endpoints in a plane
    /// perpendicular to the given Edge. Returns the positions of the resulting
    /// internal points.
    std::vector<Point3f> ApplyProfileBetweenPoints_(const PolyMesh::Edge &edge,
                                                    const Point3f &p0,
                                                    const Point3f &p1);

    /// Returns a vector of EdgeProfile_ indices in the correct direction. Does
    /// not include the first one if skip_first is true.
    Beveler2_::IndexVec_ GetEdgeProfileIndices_(const EdgeProfile_ &ep,
                                                bool skip_first = false) const;

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

    // Add faces joining profile points.
    AddVertexFaces_();
    AddEdgeFaces_();
}

PolyMesh Beveler2_::GetResultPolyMesh() {
    const PolyMesh result_mesh = pmb_.BuildPolyMesh();

    // XXXX
    {
        Debug::Dump3dv dump("/tmp/RMESH.3dv", "Result Beveler2");
        dump.SetLabelFontSize(20);
        Debug::Dump3dv::LabelFlags label_flags;
        label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
        dump.SetLabelFlags(label_flags);
        //dump.SetExtraPrefix("M_");
        //dump.AddPolyMesh(mesh);
        //dump.SetExtraPrefix("R_");
        dump.AddPolyMesh(result_mesh);
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
    auto add_border = [&](const PolyMesh::EdgeVec &edges){
        PolyMesh::IndexVec indices;
        for (const auto &edge: edges) {
            const size_t svi = GetSkeletonVertex_(*edge);

            // Add the new vertex to the offset face.
            const Point3f &p0 = edge->v0->point;
            const Point3f &p1 = skeleton_.GetVertices()[svi].point;
            const size_t index = pmb_.AddVertex(p0 + bevel_.scale * (p1 - p0));
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
        AssignEdgeDirection_(*edge, Direction_::kLeftToRight);

    // Set up the indices in the EdgeProfile_ for each PolyMesh edge, adding
    // interior profile points if necessary.
    for (auto &edge: mesh_.edges)
        SetEdgeProfileIndices_(*edge);
}

void Beveler2_::AssignEdgeDirection_(const PolyMesh::Edge &edge,
                                     Direction_ dir) {
    // Do nothing if already assigned.
    EdgeProfile_ &ep = edge_profile_map_.at(&edge);
    if (ep.direction == Direction_::kUnknown) {
        EdgeProfile_ &opp_ep  = edge_profile_map_.at(edge.opposite_edge);
        Direction_    opp_dir = GetOppositeDirection_(dir);
        ep.direction     = dir;
        opp_ep.direction = opp_dir;

        // Go to neighbors and try to apply a consistent direction.
        AssignEdgeDirection_(edge.NextEdgeInFace(), dir);
        AssignEdgeDirection_(edge.opposite_edge->NextEdgeInFace(), opp_dir);
    }
}

void Beveler2_::SetEdgeProfileIndices_(const PolyMesh::Edge &edge) {
    EdgeProfile_ &ep = edge_profile_map_.at(&edge);

    const auto &next_edge = edge.NextEdgeAroundVertex();

    ep.end0_index = edge_vertex_map_.at(&edge);
    ep.end1_index = edge_vertex_map_.at(&next_edge);

    // Insert interior vertices.
    const size_t count = bevel_.profile.GetPointCount() - 2U;
    if (count > 0U) {
        size_t i0, i1;
        if (ep.direction == Direction_::kLeftToRight) {
            i0 = ep.end0_index;
            i1 = ep.end1_index;
        }
        else {
            i0 = ep.end1_index;
            i1 = ep.end0_index;
        }
        // Store copies of the points here (not references); adding new
        // vertices could screw up references.
        const auto p0 = pmb_.GetVertex(i0);
        const auto p1 = pmb_.GetVertex(i1);

        // Apply the profile and add all internal points.
        const auto internal_points = ApplyProfileBetweenPoints_(edge, p0, p1);
        for (size_t i = 0; i < internal_points.size(); ++i) {
            const size_t index = pmb_.AddVertex(internal_points[i]);
            if (i == 0)
                ep.interior_index = index;
        }
    }
}

std::vector<Point3f> Beveler2_::ApplyProfileBetweenPoints_(
    const PolyMesh::Edge &edge, const Point3f &p0, const Point3f &p1) {
    // Compute the base point, which is the closest point on the PolyMesh edge
    // to either p0 or p1.
    const Point3f base_point =
        GetClosestPointOnLine(p0, edge.v0->point,
                              edge.v1->point - edge.v0->point);

    // Compute the vectors from the base point to p0 and p1.
    const Vector3f vec0 = p0 - base_point;
    const Vector3f vec1 = p1 - base_point;

    // Create all internal profile points.
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

void Beveler2_::AddVertexFaces_() {
    // Use a set to determine which vertices have been processed, since
    // multiple edges begin at any PolyMesh vertex.
    std::unordered_set<const PolyMesh::Vertex *> processed_vertices;

    for (const auto &edge: mesh_.edges) {
        if (! Util::MapContains(processed_vertices, edge->v0)) {
            processed_vertices.insert(edge->v0);

            // Get all edges around the vertex.
            const auto vertex_edges = PolyMesh::GetVertexEdges(*edge);

            // Collect indices of all PolyMeshBuilder vertices around the
            // vertex. Since the first vertex of each profile is the same as
            // the last vertex of the previous one, skip the first vertex for
            // each, since the vector is reversed at the end.
            IndexVec_ indices;
            for (const auto &vertex_edge: vertex_edges) {
                const auto &ep = edge_profile_map_.at(vertex_edge);
                Util::AppendVector(GetEdgeProfileIndices_(ep, true), indices);
            }
            // Edges are traversed clockwise, so have to reverse indices.
            std::reverse(indices.begin(), indices.end());

            AddVertexFaces_(vertex_edges, indices);
        }
    }
}

void Beveler2_::AddVertexFaces_(const EdgeVec_ &edges,
                                const IndexVec_ &indices) {
    // If NE is the number of edges and NP is the number of profile points,
    // then the number of points in a ring of vertices around the vertex is:
    //    NE * (NP - 1).
    const size_t edge_count   = edges.size();
    const size_t profile_size = bevel_.profile.GetPointCount();
    ASSERT(indices.size() == edge_count * (profile_size - 1));

    // Special case for default profile.
    if (profile_size == 2U) {
        pmb_.AddPolygon(indices);
    }

    // Special case for only one interior profile point. Add center points as a
    // polygon and add triangles from vertices to them.
    else if (profile_size == 3U) {
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

    // Larger case: compute inner ring points and recurse.
    else {
        RingVec_ rings = BuildRings_(edge_count, indices);
        PositionRingPoints_(edges, rings);

#if XXXX
        // Starting from the outside (first ring), connect the vertices between
        // each pair of rings.
        // XXXX N quads per edge, where N = NP for inner ring.
        for (size_t r = 0; r + 1 < rings.size(); ++r) {
            const Ring_ &outer = rings[r];
            const Ring_ &inner = rings[r + 1];
            for (size_t e = 0; e < edge_count; ++e) {
                const auto ostart = e * (outer.points_per_edge - 1);
                const auto istart = e * (inner.points_per_edge - 1);
                for (size_t p = 0; p < inner.points_per_edge; ++p) {
                    const auto i0 = outer.GetIndex[ostart + p);
                    const auto i1 = outer.GetIndex(ostart + p - 1);
                    const auto i2 = inner.GetIndex(istart + p);
                    const auto i3 = outer.GetIndex(ostart + p + 1);
                    std::cerr << "XXXX Quad " << i0
                              << " " << i1
                              << " " << i2
                              << " " << i3
                              << "\n";
                    //                    pmb_.AddQuad(i0, i1, i2, i3);
                }
            }
        }
#endif
    }
}

Beveler2_::RingVec_ Beveler2_::BuildRings_(size_t edge_count,
                                           const IndexVec_ &outer_indices) {
    // Compute the number of rings that are needed. Each ring has 2 fewer
    // points per edge than the ring surrounding it.
    const size_t profile_size = bevel_.profile.GetPointCount();
    const size_t ring_count   = profile_size / 2;
    std::cerr << "XXXX ======== NE = " << edge_count
              << " NP = " << profile_size
              << " NR = " << ring_count << "\n";
    RingVec_ rings(ring_count);

    for (size_t r = 0; r < ring_count; ++r) {
        Ring_ &ring = rings[r];
        ring.level = r;
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
        std::cerr << "XXXX Ring level " << ring.level
                  << " with PPS = " << ring.points_per_side
                  << " and " << ring.indices.size() << " points\n";
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
    const size_t edge_count = edges.size();

    for (size_t i = 0; i < rings.size(); ++i) {
        Ring_ &ring = rings[i];
        ring.reposition_counts.assign(ring.indices.size(), 0);
    }

    const auto prev_side = [edge_count](size_t side){
        return (side + edge_count - 1) % edge_count;
    };
    const auto next_side = [edge_count](size_t side){
        return (side + 1) % edge_count;
    };

    // Work towards the innermost ring.
    const Ring_  &outer_ring = rings[0];
    const size_t  outer_pps  = outer_ring.points_per_side;
    for (size_t r = 1; r < rings.size(); ++r) {
        Ring_ &ring = rings[r];
        for (size_t side = 0; side < edge_count; ++side) {
            // Get the endpoints from the outermost ring for applying the
            // profile. The first endpoint is the next-to-last point on the
            // previous side.
            const size_t i0 = outer_ring.GetSideIndex(prev_side(side),
                                                      outer_pps - 2);
            const size_t i1 = outer_ring.GetSideIndex(next_side(side), 1);
            const Point3f &p0 = pmb_.GetVertex(outer_ring.GetVertexIndex(i0));
            const Point3f &p1 = pmb_.GetVertex(outer_ring.GetVertexIndex(i1));

            // Apply the profile to get the interior points.
            const auto pts = ApplyProfileBetweenPoints_(*edges[side], p0, p1);

            // Store the appropriate points in the ring.
            for (size_t i = 0; i < ring.points_per_side - 1; ++i) {
                const Point3f &pos = pts[r + i];
                const size_t index   = ring.GetSideIndex(side, i);
                const size_t v_index = ring.GetVertexIndex(index);
                size_t &count = ring.reposition_counts[index];
                std::cerr << "XXXX Ring " << r
                          << " repos [side=" << side << ", i=" << i
                          << "] count = " << count
                          << " index=" << index
                          << " pos=" << pos
                          << "\n";
                if (count == 0U) {
                    pmb_.MoveVertex(v_index, pos);
                }
                else {
                    ASSERT(count == 1U);
                    pmb_.MoveVertex(v_index,
                                    .5f * (pos + pmb_.GetVertex(v_index)));
                }
                ++count;
            }
        }
    }
}

void Beveler2_::AddEdgeFaces_() {
    // This set is used to keep edges from being processed twice when the
    // opposite edge is added.
    std::unordered_set<const PolyMesh::Edge *> processed_edges;
    for (const auto &edge: mesh_.edges) {
        if (! Util::MapContains(processed_edges, edge)) {
            processed_edges.insert(edge);
            processed_edges.insert(edge->opposite_edge);

            const auto &ep0 = edge_profile_map_.at(edge);
            const auto &ep1 = edge_profile_map_.at(edge->opposite_edge);

            // Special case for default profile (2 points).
            if (bevel_.profile.GetPointCount() == 2U) {
                pmb_.AddQuad(ep0.end0_index, ep0.end1_index,
                             ep1.end0_index, ep1.end1_index);
            }
            else {
                const auto indices0 = GetEdgeProfileIndices_(ep0);
                auto indices1       = GetEdgeProfileIndices_(ep1);
                std::reverse(indices1.begin(), indices1.end());
                ASSERT(indices0.size() == indices1.size());
                for (size_t i = 1; i < indices0.size(); ++i) {
                    pmb_.AddQuad(indices0[i - 1], indices0[i],
                                 indices1[i],     indices1[i - 1]);
                }
            }
        }
    }
}

 Beveler2_::IndexVec_ Beveler2_::GetEdgeProfileIndices_(const EdgeProfile_ &ep,
                                                        bool skip_first) const {
    const size_t profile_count = bevel_.profile.GetPointCount();
    ASSERT(profile_count >= 2U);

    IndexVec_ indices;
    indices.reserve(profile_count);

    if (! skip_first)
        indices.push_back(ep.end0_index);
    const size_t interior_count = profile_count - 2;
    if (ep.direction == Direction_::kLeftToRight) {
        for (size_t i = 0; i < interior_count; ++i)
            indices.push_back(ep.interior_index + i);
    }
    else {
        for (size_t i = 0; i < interior_count; ++i)
            indices.push_back(ep.interior_index + interior_count - (i + 1));
    }
    indices.push_back(ep.end1_index);

    return indices;
}

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
