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

    /// Returns the index of the ith vertex in the Profile assuming there are N
    /// vertices.
    size_t GetIndex(size_t i, size_t n) {
        return i == 0 ? end0_index : i + 1 == n ? end1_index :
            interior_index + i - 1;
    }
};

/// Struct encapsulating data needed for dealing with a Skeleton3D.
struct SkelData_ {
    Skeleton3D skeleton;

    /// Maps a PolyMesh::Vertex pointer to a skeleton vertex index.
    std::unordered_map<const PolyMesh::Vertex *, size_t> vertex_map;
};

// ----------------------------------------------------------------------------
// Beveler2_ class. XXXX
// ----------------------------------------------------------------------------

class Beveler2_ {
  public:
    /// Applies the Bevel and returns the resulting PolyMesh.
    PolyMesh ApplyBevel(const PolyMesh &mesh, const Bevel &bevel);

  private:
    /// Maps a PolyMesh::Edge pointer to the PolyMeshBuilder index of the new
    /// offset vertex created for it.
    typedef std::unordered_map<const PolyMesh::Edge *, size_t> EVMap_;

    /// Maps a PolyMesh::Edge pointer to the EdgeProfile__ created for it.
    typedef std::unordered_map<const PolyMesh::Edge *, EdgeProfile_> EPMap_;

    /// XXXX
    void InitSkeleton_(const PolyMesh &mesh, SkelData_ &skel_data);

    /// Returns the index of the skeleton vertex at the other end of the
    /// bisector edge from the starting vertex of the given PolyMesh::Edge
    /// within its face. Asserts if it is not found.
    size_t GetSkeletonVertex_(const PolyMesh::Edge &edge,
                              const SkelData_ &skel_data);

    /// XXXX
    void AddOffsetFaceVertices_(const PolyMesh &mesh, float offset,
                                const SkelData_ &skel_data,
                                PolyMeshBuilder &pmb,
                                EVMap_ &edge_vertex_map);

    /// XXXX
    void CreateEdgeProfiles_(const PolyMesh &mesh,
                             const EVMap_ &edge_vertex_map,
                             PolyMeshBuilder &pmb,
                             EPMap_ &edge_profile_map);

    /// Assigns a direction to an edge of the original PolyMesh and its
    /// opposite edge, moving on to neighbors as well.
    void AssignEdgeDirection_(const PolyMesh::Edge &edge, Direction_ dir,
                              EPMap_ &edge_profile_map);

    /// XXXX
    void SetEdgeProfileIndices_(const PolyMesh::Edge &edge,
                                const EVMap_ &edge_vertex_map,
                                PolyMeshBuilder &pmb, EdgeProfile_ &ep);

    /// Adds faces joining all profile points around each original vertex of
    /// the PolyMesh.
    void AddVertexFaces_(const PolyMesh &mesh, const EPMap_ &edge_profile_map,
                         PolyMeshBuilder &pmb);

    /// Adds faces joining all profile points across each original edge of the
    /// PolyMesh.
    void AddEdgeFaces_(const PolyMesh &mesh, const EPMap_ &edge_profile_map,
                       PolyMeshBuilder &pmb);

    /// XXXX
    void DumpSkeleton_(const PolyMesh &mesh, const Skeleton3D &skeleton);
};

PolyMesh Beveler2_::ApplyBevel(const PolyMesh &mesh, const Bevel &bevel) {
    // Set up the Skeleton3D and map for its vertices.
    SkelData_ skel_data;
    InitSkeleton_(mesh, skel_data);

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    PolyMeshBuilder pmb;

    // Using the skeleton, create offset vertices for each face of the PolyMesh
    // and add the vertices to the PolyMeshBuilder. Store the resulting indices
    // in a map keyed by edge pointer.
    EVMap_ edge_vertex_map;
    AddOffsetFaceVertices_(mesh, bevel.scale, skel_data, pmb, edge_vertex_map);

    // Set up EdgeProfile_ instances for all PolyMesh edges.
    EPMap_ edge_profile_map;
    CreateEdgeProfiles_(mesh, edge_vertex_map, pmb, edge_profile_map);

    // Add faces joining profile points.
    AddVertexFaces_(mesh, edge_profile_map, pmb);
    AddEdgeFaces_(mesh,   edge_profile_map, pmb);

    const PolyMesh result_mesh = pmb.BuildPolyMesh();

    // XXXX
    Debug::Dump3dv dump("/tmp/RMESH.3dv", "Result Beveler2");
    dump.SetLabelFontSize(40);
    dump.AddPolyMesh(result_mesh);

    return result_mesh;
}

void Beveler2_::InitSkeleton_(const PolyMesh &mesh, SkelData_ &skel_data) {
    // Build the 3D straight skeleton for the PolyMesh.
    skel_data.skeleton.BuildForPolyMesh(mesh);
    DumpSkeleton_(mesh, skel_data.skeleton);  // XXXX

    // Map PolyMesh vertices to skeleton vertices.
    const auto &skel_vertices = skel_data.skeleton.GetVertices();
    for (size_t i = 0; i < skel_vertices.size(); ++i) {
        const int index = skel_vertices[i].source_index;
        // Add only skeleton vertices that correspond to PolyMesh vertices.
        if (index >= 0) {
            ASSERT(static_cast<size_t>(index) < mesh.vertices.size());
            const auto &pmv = mesh.vertices[index];
            ASSERT(! Util::MapContains(skel_data.vertex_map, pmv));
            skel_data.vertex_map[pmv] = i;
        }
    }
    ASSERT(skel_data.vertex_map.size() == mesh.vertices.size());
}

size_t Beveler2_::GetSkeletonVertex_(const PolyMesh::Edge &edge,
                                     const SkelData_ &skel_data) {
    // Get the starting vertex for the Edge and the previous and next vertices
    // in the same Face.
    const auto &vstart = edge.v0;
    const auto &vnext  = edge.v1;
    const auto &vprev  = edge.PreviousEdgeInFace().v0;
    const size_t skel_vstart = skel_data.vertex_map.at(vstart);
    const size_t skel_vnext  = skel_data.vertex_map.at(vnext);
    const size_t skel_vprev  = skel_data.vertex_map.at(vprev);

    // Find the skeleton bisector for the starting vertex of the face.
    // XXXX Speed this up?
    const int vni = static_cast<int>(skel_vnext);
    const int vpi = static_cast<int>(skel_vprev);
    for (const auto &edge: skel_data.skeleton.GetEdges()) {
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

void Beveler2_::AddOffsetFaceVertices_(const PolyMesh &mesh, float offset,
                                       const SkelData_ &skel_data,
                                       PolyMeshBuilder &pmb,
                                       EVMap_ &edge_vertex_map) {
    auto add_border = [&](const PolyMesh::EdgeVec &edges){
        PolyMesh::IndexVec indices;
        for (const auto &edge: edges) {
            const size_t svi = GetSkeletonVertex_(*edge, skel_data);

            // Add the new vertex to the offset face.
            const Point3f &p0 = edge->v0->point;
            const Point3f &p1 = skel_data.skeleton.GetVertices()[svi].point;
            const size_t index = pmb.AddVertex(p0 + offset * (p1 - p0));
            indices.push_back(index);

            // Store the correspondence from the original edge to the new
            // vertex.
            edge_vertex_map[edge] = index;
        }
        // Need to reverse the order to maintain orientation.
        std::reverse(indices.begin(), indices.end());
        return indices;
    };

    for (const auto &face: mesh.faces) {
        pmb.AddPolygon(add_border(face->outer_edges));
        for (auto &hole: face->hole_edges)
            pmb.AddHole(add_border(hole));
    }
}

void Beveler2_::CreateEdgeProfiles_(const PolyMesh &mesh,
                                    const EVMap_ &edge_vertex_map,
                                    PolyMeshBuilder &pmb,
                                    EPMap_ &edge_profile_map) {
    // Create a default EdgeProfile_ instance for each PolyMesh edge.
    for (const auto &edge: mesh.edges)
        edge_profile_map[edge] = EdgeProfile_();

    // Assign directions to the EdgeProfile_ instances.
    for (auto &edge: mesh.edges)
        AssignEdgeDirection_(*edge, Direction_::kLeftToRight, edge_profile_map);

    // Set up the indices in the EdgeProfile_ for each PolyMesh edge, adding
    // interior profile points if necessary.
    for (auto &edge: mesh.edges)
        SetEdgeProfileIndices_(*edge, edge_vertex_map, pmb,
                               edge_profile_map.at(edge));
}

void Beveler2_::AssignEdgeDirection_(const PolyMesh::Edge &edge, Direction_ dir,
                                     EPMap_ &edge_profile_map) {
    const auto get_opp_dir = [](Direction_ d){
        return d == Direction_::kLeftToRight ?
            Direction_::kRightToLeft : Direction_::kLeftToRight;
    };

    // Do nothing if already assigned.
    EdgeProfile_ &ep = edge_profile_map.at(&edge);
    if (ep.direction == Direction_::kUnknown) {
        EdgeProfile_ &opp_ep  = edge_profile_map.at(edge.opposite_edge);
        Direction_    opp_dir = get_opp_dir(ep.direction);
        ep.direction     = dir;
        opp_ep.direction = opp_dir;

        // Go to neighbors and try to apply a consistent direction.
        AssignEdgeDirection_(edge.NextEdgeInFace(), dir, edge_profile_map);
        AssignEdgeDirection_(edge.opposite_edge->NextEdgeInFace(),
                             opp_dir, edge_profile_map);
    }
}

void Beveler2_::SetEdgeProfileIndices_(const PolyMesh::Edge &edge,
                                       const EVMap_ &edge_vertex_map,
                                       PolyMeshBuilder &pmb, EdgeProfile_ &ep) {
    const auto &next_edge = edge.NextEdgeAroundVertex();

    // XXXX Assume just 2-point bevel for now...
    ep.end0_index = edge_vertex_map.at(&edge);
    ep.end1_index = edge_vertex_map.at(&next_edge);
}

void Beveler2_::AddVertexFaces_(const PolyMesh &mesh,
                                const EPMap_ &edge_profile_map,
                                PolyMeshBuilder &pmb) {
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
            std::reverse(indices.begin(), indices.end());
            pmb.AddPolygon(indices);

            // Connect 
        }
    }
#endif
}

void Beveler2_::AddEdgeFaces_(const PolyMesh &mesh,
                              const EPMap_ &edge_profile_map,
                              PolyMeshBuilder &pmb) {
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
    Beveler2_ beveler;
    return beveler.ApplyBevel(poly_mesh, bevel).ToTriMesh();
}
