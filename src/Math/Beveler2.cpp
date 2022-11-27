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

    /// Adds faces joining all profile points across each original edge of the
    /// PolyMesh.
    void AddEdgeFaces_();

    /// Returns a vector of EdgeProfile_ indices in the correct direction.
    std::vector<size_t> GetEdgeProfileIndices_(const EdgeProfile_ &ep) const;

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
        dump.SetLabelFontSize(40);
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
        // Store copies of the points here (not references); adding new
        // vertices could screw up references.
        const auto p0 = pmb_.GetVertex(ep.end0_index);
        const auto p1 = pmb_.GetVertex(ep.end1_index);

        for (size_t i = 0; i < count; ++i) {
            // XXXX Do real math here...
            const float frac = static_cast<float>(i + 1) / (count + 1);
            const size_t index = pmb_.AddVertex(p0 + frac * (p1 - p0));
            if (i == 0)
                ep.interior_index = index;
        }
    }
}

void Beveler2_::AddVertexFaces_() {
#if XXXX
    // Use a set to determine which vertices have been processed.
    std::unordered_set<const PolyMesh::Vertex *> processed_vertices;

    for (const auto &edge: mesh.edges) {
        if (! Util::MapContains(processed_vertices, edge->v0)) {
            processed_vertices.insert(edge->v0);

            // Access all Chain_ instances. Note that index1 in each chain
            // should be the same as index0 in the next chain.
            std::vector<Chain_> chains =
                Util::ConvertVector<Chain_, PolyMesh::Edge *>(
                    PolyMesh::GetVertexEdges(*edge),
                    [&](const PolyMesh::Edge *e){ return chain_map.at(e); });

            // XXXX Assume 3 points in each Chain_.
            const size_t n = 3;  // XXXX

            // Process all rings and then the innermost polygon). Start on the
            // outside and work in. This depends on whether N is even or odd:
            // For example, if N is 4, there are rings between points 0 and 1
            // and 2 and 3, and the innermost polygon is between points 1 and
            // 2. If N is 3, there are rings between points 0 and 1, 1 and 2, 

            // Process rings (all but the innermost polygon). Start on the
            // outside and work in. For example, if N is 4, there are rings
            // between points 0 and 1 and 1 and 2
            for (size_t i = 1; i < n; ++i) {
                // Collect vertices in the outer and 
                XXXXXXXXXXXX;

                // XXXX Need to assign direction to each edge FIRST.

            // Add the innermost polygon.


            // Add triangles between each pair of chain points.
                // Collect inner and outer chain points.
                std::vector<GIndex> inner, outer;

                for (size_t c = 0; c < chains.size(); ++c) {
                }
            }

            // Connect interior points. Need to reverse them.
            std::vector<GIndex> indices = Util::ConvertVector<GIndex, Chain_>(
                chains, [](const Chain_ c){ return c.interior_index; });
            pmb.AddPolygon(indices);
        }
    }
#endif
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

std::vector<size_t> Beveler2_::GetEdgeProfileIndices_(
    const EdgeProfile_ &ep) const {
    const size_t profile_size = bevel_.profile.GetPointCount();
    ASSERT(profile_size >= 2U);
    std::vector<size_t> indices;
    indices.push_back(ep.end0_index);
    for (size_t i = 0; i < profile_size - 2; ++i)
        indices.push_back(ep.interior_index + i);
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
