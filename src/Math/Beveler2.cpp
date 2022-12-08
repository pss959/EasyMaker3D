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
/// starting vertex (v0) of an edge of the original PolyMesh. The \c direction
/// field is used to maintain the proper vertex order. The \c angle field is
/// used to determine whether the edge passes the max-angle test.  The points
/// of the EdgeProfile_ are added as vertices in a PolyMeshBuilder and the
/// resulting indices are stored here in the \c indices field. Profile vertices
/// are always ordered in a counterclockwise direction around the v0 vertex.
struct EdgeProfile_ {
    Direction_          direction = Direction_::kUnknown;
    Anglef              angle;
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
    Beveler2_(const PolyMesh &mesh, const Bevel &bevel, PolyMesh &result_mesh);

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
    std::unordered_map<const PolyMesh::Edge *, GIndex>       edge_vertex_map_;

    /// Maps each PolyMesh::Edge pointer to the EdgeProfile_ created for it.
    std::unordered_map<const PolyMesh::Edge *, EdgeProfile_> edge_profile_map_;

    /// XXXX Comment all of this...
    void CreateEdgeProfiles_();

    /// Assigns a direction to an edge of the original PolyMesh and its
    /// opposite edge, moving on to neighbors as well. Also computes and stores
    /// the angle between the faces adjacent to the edges.
    void AssignEdgeDirectionAndAngle_(const PolyMesh::Edge &edge,
                                      Direction_ dir);

    void InitSkeleton_();

    /// Returns the index of the skeleton vertex at the other end of the
    /// bisector edge from the starting vertex of the given PolyMesh::Edge
    /// within its face. Asserts if it is not found.
    size_t GetSkeletonVertex_(const PolyMesh::Edge &edge);

    IndexVec_ AddOffsetFaceBorder_(const EdgeVec_ &border_edges,
                                   const Vector3f &normal, bool is_hole);

    Point3f ComputeOffsetPosition_(const PolyMesh::Edge &e0,
                                   const PolyMesh::Edge &e1,
                                   const Vector3f &normal, bool is_hole);

    void CollapseEdgeVertices_();

    void SetEdgeProfileIndices_(const PolyMesh::Edge &edge);

    /// Adds faces joining all profile points across each original edge of the
    /// PolyMesh.
    void AddEdgeFaces_();

    /// Adds faces joining all profile points around each original vertex of
    /// the PolyMesh.
    void AddVertexFaces_();

    void AddVertexFaces_(const EdgeVec_ &edges, const IndexVec_ &indices);
    void AddInnerVertexFaces_(const IndexVec_ &indices,
                              size_t points_per_side);
    Beveler2_::RingVec_ BuildRings_(const EdgeVec_ &edges,
                                    const IndexVec_ &outer_indices);
    void PositionRingPoints_(const EdgeVec_ &edges, RingVec_ &rings);
    void AddInterRingFaces_(const RingVec_ &rings);

    /// Applies the bevel profile between the two given endpoints in a plane
    /// perpendicular to the given Edge. Returns the positions of the resulting
    /// interior points.
    std::vector<Point3f> ApplyProfileBetweenPoints_(const PolyMesh::Edge &edge,
                                                    const Point3f &p0,
                                                    const Point3f &p1);

    void DumpSkeleton_(const PolyMesh &mesh, const Skeleton3D &skeleton);
};

Beveler2_::Beveler2_(const PolyMesh &mesh, const Bevel &bevel,
                     PolyMesh &result_mesh) :
    mesh_(mesh),
    bevel_(bevel) {

    // Set up EdgeProfile_ instances for all PolyMesh edges and store them in
    // the edge_profile_map_.
    CreateEdgeProfiles_();

    // Set up the Skeleton3D and map for its vertices.
    InitSkeleton_();

    // Using the skeleton, create offset vertices for each face of the PolyMesh
    // and add the vertices to the PolyMeshBuilder. Store the resulting indices
    // in a the edge_vertex_map_ and create faces joining them.
    for (const auto &face: mesh_.faces) {
        const Vector3f normal = face->GetNormal();
        pmb_.AddPolygon(AddOffsetFaceBorder_(face->outer_edges, normal, false));
        for (auto &hole: face->hole_edges)
            pmb_.AddHole(AddOffsetFaceBorder_(hole, normal, true));
    }

    // Collapse vertices for edges that exceed the maximum bevel angle.
    CollapseEdgeVertices_();

    // Set the indices in the EdgeProfile_ for each PolyMesh edge, adding
    // interior profile points to the PolyMeshBuilder if necessary.
    for (auto &edge: mesh_.edges)
        SetEdgeProfileIndices_(*edge);

    // Add faces joining profile points across edges and around vertices.
    AddEdgeFaces_();
    AddVertexFaces_();

    // Construct the result PolyMesh and make sure the resulting PolyMesh has
    // no duplicate features.
    pmb_.BuildPolyMesh(result_mesh);
    {  // XXXX
        const bool add_orig_mesh = true;

        Debug::Dump3dv dump("/tmp/UMESH.3dv", "Unmerged Beveler2");
        dump.SetLabelFontSize(8);
        Debug::Dump3dv::LabelFlags label_flags;
        label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
        //label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
        //label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
        dump.SetLabelFlags(label_flags);
        //dump.AddPolyMesh(result_mesh);
        if (add_orig_mesh) {
            // dump.SetExtraPrefix("M_");
            label_flags.Reset(Debug::Dump3dv::LabelFlag::kVertexLabels);
            label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
            label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
            dump.SetLabelFlags(label_flags);
            dump.AddPolyMesh(mesh_);
        }
    }
    MergeDuplicateFeatures(result_mesh, result_mesh);
}

void Beveler2_::CreateEdgeProfiles_() {
    // Create a default EdgeProfile_ instance for each PolyMesh edge.
    for (const auto &edge: mesh_.edges)
        edge_profile_map_[edge] = EdgeProfile_();

    // Assign directions and angles in the EdgeProfile_ instances, recursively.
    for (auto &edge: mesh_.edges)
        AssignEdgeDirectionAndAngle_(*edge, Direction_::kForward);
}

void Beveler2_::AssignEdgeDirectionAndAngle_(const PolyMesh::Edge &edge,
                                             Direction_ dir) {
    // Do nothing if already assigned.
    EdgeProfile_ &ep = edge_profile_map_.at(&edge);
    if (ep.direction != Direction_::kUnknown)
        return;

    ASSERT(edge.opposite_edge);
    const auto      &opp_edge = *edge.opposite_edge;
    EdgeProfile_    &opp_ep   = edge_profile_map_.at(&opp_edge);
    const Direction_ opp_dir  = GetOppositeDirection_(dir);
    ep.direction     = dir;
    opp_ep.direction = opp_dir;

    // Compute and store the angle formed by the edge's faces.
    const Vector3f &normal0 = edge.face->GetNormal();
    const Vector3f &normal1 = edge.opposite_edge->face->GetNormal();
    ep.angle = opp_ep.angle = Anglef::FromDegrees(180) -
        ion::math::AngleBetween(normal0, normal1);

    // Collapsed edges do not affect any others. For other edges, go to
    // neighbors and try to apply a consistent direction.
    if (ep.angle <= bevel_.max_angle) {
        AssignEdgeDirectionAndAngle_(edge.NextEdgeInFace(),     dir);
        AssignEdgeDirectionAndAngle_(opp_edge.NextEdgeInFace(), opp_dir);
    }
}

void Beveler2_::InitSkeleton_() {
    // Build the 3D straight skeleton for the PolyMesh.
    skeleton_.BuildForPolyMesh(mesh_);

    // Create a map from PolyMesh vertices to skeleton vertices.
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

Beveler2_::IndexVec_ Beveler2_::AddOffsetFaceBorder_(
    const EdgeVec_ &border_edges, const Vector3f &normal, bool is_hole) {

    IndexVec_ indices;
    for (size_t i = 0; i < border_edges.size(); ++i) {
        // Get the two border edges around the current point. They meet at
        // e0.v1 == e1.v0.
        const auto &e0 = *border_edges[i];
        const auto &e1 = *border_edges[(i + 1) % border_edges.size()];
        const auto &ep0 = edge_profile_map_.at(&e0);
        const auto &ep1 = edge_profile_map_.at(&e1);
        ASSERT(e0.v1 == e1.v0);
        const Point3f p0 = e1.v0->point;

        // Determine if the edges should be beveled.
        const bool is_e0_beveled = ep0.angle <= bevel_.max_angle;
        const bool is_e1_beveled = ep1.angle <= bevel_.max_angle;

        Point3f pos;
        if (is_e0_beveled && is_e1_beveled) {
            pos = ComputeOffsetPosition_(e0, e1, normal, is_hole);
        }
        else if (is_e0_beveled && ! is_e1_beveled) {
            pos = p0 - bevel_.scale * e0.GetUnitVector();
        }
        else if (! is_e0_beveled && is_e1_beveled) {
            pos = p0 + bevel_.scale * e1.GetUnitVector();
        }
        else {
            pos = e1.v0->point;
        }

        // Add the new vertex to the offset face.
        const size_t index = pmb_.AddVertex(pos);
        indices.push_back(index);

        // Store the correspondence from the original edge to the new
        // vertex.
        edge_vertex_map_[&e1] = index;
    }
    return indices;
}

Point3f Beveler2_::ComputeOffsetPosition_(const PolyMesh::Edge &e0,
                                          const PolyMesh::Edge &e1,
                                          const Vector3f &normal,
                                          bool is_hole) {
    using ion::math::AngleBetween;
    using ion::math::Normalized;
    using ion::math::Sine;

    const Vector3f e0_vec = -e0.GetUnitVector();
    const Vector3f e1_vec =  e1.GetUnitVector();

    // Method A:

    // Get the corresponding skeleton vertex.
    const size_t svi = GetSkeletonVertex_(e1);
    const Point3f &p0  = e1.v0->point;
    const Point3f &p1 = skeleton_.GetVertices()[svi].point;

    // Use trigonometry to find the position P along the skeleton bisector edge
    // that is S units from either of the original edges, where S is the bevel
    // scale.
    const Vector3f bisectorA = Normalized(p1 - p0);
    const Anglef   angleA    = AngleBetween(e1_vec, bisectorA);
    const float    distA     = bevel_.scale / Sine(angleA);
    const Point3f  posA      = p0 + distA * bisectorA;

    // Method B:

    const Vector3f cross = ion::math::Cross(e0_vec, e1_vec);
    const float    dot   = ion::math::Dot(cross, normal);
    const bool flip = is_hole ? dot < 0 : dot > 0;

    const Vector3f bisectorB = (flip ? -1 : 1) * Normalized(e0_vec + e1_vec);
    const Anglef   angleB    = AngleBetween(e1_vec, bisectorB);
    const float    distB     = bevel_.scale / Sine(angleB);
    const Point3f  posB      = p0 + distB * bisectorB;

    const float diff = ion::math::Distance(posA, posB);
    if (diff > .001f) {
        std::cerr << "XXXX For " << e0.id
                  << " and " << e1.id << ":\n"
                  << "  e0_vec = " << e0_vec << "\n"
                  << "  e1_vec = " << e1_vec << "\n"
                  << "  is_hole=" << is_hole << "\n"
                  << "  norm   = " << normal << "\n"
                  << "  cross  = " << cross << "\n"
                  << "  dot    = " << dot << "\n"
                  << "  flip   = " << flip << "\n"
                  << "  bisA   = " << bisectorA << "\n"
                  << "  bisB   = " << bisectorB << "\n"
                  << "  angA   = " << angleA.Degrees() << "\n"
                  << "  angB   = " << angleB.Degrees() << "\n"
                  << "  e ang  = " << AngleBetween(e0_vec,
                                                   e1_vec).Degrees() << "\n"
                  << "  2 angA = " << (2 * angleA.Degrees()) << "\n"
                  << "  2 angB = " << (2 * angleB.Degrees()) << "\n"
                  << "  distA  = " << distA << "\n"
                  << "  distB  = " << distB << "\n"
                  << "  diff   = " << diff
                  << "\n";
    }
#if XXXX
    else {
        std::cerr << "XXXX For " << e0.id
                  << " and " << e1.id << ":\n"
                  << "  e0_vec = " << e0_vec << " e1_vec = " << e1_vec
                  << " is_hole=" << is_hole << "\n"
                  << "  e ang  = "
                  << AngleBetween(e0_vec, e1_vec).Degrees() << "\n"
                  << "  cross  = "
                  << ion::math::Cross(e0_vec, e1_vec) << "\n";
    }
#endif

    // return posB;
    return posA;
}

void Beveler2_::CollapseEdgeVertices_() {
    for (const auto &edge: mesh_.edges) {
        auto &ep = edge_profile_map_.at(edge);

        if (ep.angle > bevel_.max_angle) {
            const auto collapse = [&](const PolyMesh::Edge &e){
                // Move the edge's offset vertex to the closest point on the
                // collapsed edge.
                const GIndex index = edge_vertex_map_.at(&e);
                pmb_.MoveVertex(index, GetClosestPointOnLine(
                                    pmb_.GetVertex(index),
                                    edge->v0->point, edge->GetUnitVector()));
            };

            // Collapse the offset vertex created for the given edge and also
            // the vertex created for the next edge in the face, which is on
            // the same offset edge.
            collapse(*edge);
            collapse(edge->NextEdgeInFace());
        }
    }
}

void Beveler2_::SetEdgeProfileIndices_(const PolyMesh::Edge &edge) {
    const size_t i0 = edge_vertex_map_.at(&edge.PreviousEdgeAroundVertex());
    const size_t i1 = edge_vertex_map_.at(&edge);

    const size_t profile_size = bevel_.profile.GetPointCount();

    EdgeProfile_ &ep = edge_profile_map_.at(&edge);
    ep.indices.reserve(profile_size);

    // Starting vertex.
    ep.indices.push_back(i0);

    // Interior vertices, if any. Reverse if necessary to maintain proper
    // direction.
    if (profile_size > 2U) {
        const Point3f p0 = pmb_.GetVertex(i0);
        const Point3f p1 = pmb_.GetVertex(i1);

        std::vector<Point3f> pts;
        // Special case if the endpoints are very close due to collapse.
        if (AreClose(p0, p1)) {
            pts.assign(profile_size - 2, p0);
        }
        else if (ep.direction == Direction_::kReverse) {
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
            const GIndex i0 = indices[i];
            const GIndex i1 = indices[(i + 1) % index_count];
            const GIndex i2 = indices[(i + 2) % index_count];
            inner_indices.push_back(i0);
            pmb_.AddTriangle(i0, i1, i2);
        }
        pmb_.AddPolygon(inner_indices);
    }
}

Beveler2_::RingVec_ Beveler2_::BuildRings_(const EdgeVec_ &edges,
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
            // If the endpoints are very close (due to collapse), just create
            // copies of the endpoints. Otherwise, apply the profile to get the
            // interior points.
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

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Beveler2 functions.
// ----------------------------------------------------------------------------

TriMesh Beveler2::ApplyBevel(const TriMesh &mesh, const Bevel &bevel) {
    // Convert the TriMesh to a PolyMesh and merge coplanar faces.
    PolyMesh poly_mesh(mesh);
    MergeCoplanarFaces(poly_mesh);

    // Use a Beveler2_ to do the rest.
    PolyMesh result_mesh;
    Beveler2_ beveler(poly_mesh, bevel, result_mesh);

    return result_mesh.ToTriMesh();
}
