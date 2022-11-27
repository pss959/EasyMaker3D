#include "Math/Beveler2.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "Math/Linear.h"
#include "Math/PolyMesh.h"
#include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Skeleton3D.h"
#include "Util/Assert.h"
#include "Util/General.h"

#include "Debug/Dump3dv.h" // XXXX
#include "Util/String.h" // XXXX

// ----------------------------------------------------------------------------
// Beveler2::Helper_ class. XXXX
// ----------------------------------------------------------------------------

class Beveler2::Helper_ {
  public:
    /// The constructor is passed the PolyMesh to bevel using the given Bevel.
    Helper_(const PolyMesh &mesh, const Bevel &bevel);

    /// Applies the Bevel and returns the resulting PolyMesh.
    PolyMesh ApplyBevel();

  private:
    /// This stores the indices of points added to the PolyMeshBuilder for the
    /// bevel profile at the start vertex of a PolyMesh::Edge.
    struct Chain_ {
        /// Index of the first point in the chain.
        size_t end0_index;
        /// Index of the last point in the chain.
        size_t end1_index;
        /// Index of the first interior point of the chain. This is unused if
        /// the profile is a simple bevel with only 2 points.
        size_t interior_index = 0;

        /// Returns the index of the ith vertex in the chain assuming there are
        /// N vertices in the chain.
        size_t GetIndex(size_t i, size_t n) {
            return i == 0 ? end0_index : i + 1 == n ? end1_index :
                interior_index + i - 1;
        }
    };

    /// Maps a PolyMesh::Vertex pointer to a skeleton vertex index.
    typedef std::unordered_map<const PolyMesh::Vertex *, size_t> SkelVMap_;

    /// Maps a PolyMesh::Edge pointer to the PolyMeshBuilder index of the new
    /// vertex created for it.
    typedef std::unordered_map<const PolyMesh::Edge *, size_t> PMEdgeMap_;

    /// Maps a PolyMesh::Edge pointer to the Chain_ created for its starting
    /// vertex.
    typedef std::unordered_map<const PolyMesh::Edge *, Chain_> ChainMap_;

    const PolyMesh &mesh_;             ///< Original PolyMesh to bevel.
    const Bevel    &bevel_;            ///< Bevel to apply.
    Skeleton3D      skeleton_;         ///< Straight skeleton of the PolyMesh.
    SkelVMap_       skel_vertex_map_;  ///< Map used to find skeleton vertices.

    /// Builds the map from PolyMesh::Vertex pointer to skeleton vertex index.
    void BuildSkeletonVertexMap_();

    /// Adds offset vertices for each vertex in the given PolyMesh::Face to the
    /// given PolyMeshBuilder, while also storing the indices for each
    /// PolyMesh::Edge in the PMEdgeMap_. Returns a vector of indices.
    PolyMesh::IndexVec AddFaceVertices_(const PolyMesh::Face &face,
                                        PolyMeshBuilder &pmb,
                                        PMEdgeMap_ &edge_vertex_map);

    /// Sets up a Chain_ for each edge joining offset points around each
    /// original vertex of the PolyMesh. The map is used to access the new
    /// vertices.
    void AddChains_(PolyMeshBuilder &pmb, const PMEdgeMap_ &edge_vertex_map,
                    ChainMap_ &chain_map);

    /// Adds faces joining all Chain_ points around each original vertex of the
    /// PolyMesh. The map is used to access the new vertices.
    void AddVertexFaces_(PolyMeshBuilder &pmb, const ChainMap_ &chain_map);

    /// Adds faces joining all Chain_ points across each original edge of the
    /// PolyMesh. The map is used to access the new vertices.
    void AddEdgeFaces_(PolyMeshBuilder &pmb, const ChainMap_ &chain_map);

    /// Returns the index of the skeleton vertex at the other end of the
    /// bisector edge from the starting vertex of the given PolyMesh::Edge
    /// within its face. Asserts if it is not found.
    size_t GetSkeletonVertex_(const PolyMesh::Edge &edge);

    /// XXXX
    void DumpSkeleton_();
};

Beveler2::Helper_::Helper_(const PolyMesh &mesh, const Bevel &bevel) :
    mesh_(mesh),
    bevel_(bevel) {
}

PolyMesh Beveler2::Helper_::ApplyBevel() {
    // Build a 3D straight skeleton for the PolyMesh.
    skeleton_.BuildForPolyMesh(mesh_);

    DumpSkeleton_();  // XXXX

    // Map PolyMesh vertices to skeleton vertices.
    BuildSkeletonVertexMap_();

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    PolyMeshBuilder pmb;

    // Maps PolyMesh::Edge pointer to the index of the new vertex created for
    // it in the PolyMeshBuilder.
    PMEdgeMap_ edge_vertex_map;

    // Add vertices for each face of the PolyMesh, adding to edge_vertex_map.
    for (const auto &face: mesh_.faces)
        pmb.AddPolygon(AddFaceVertices_(*face, pmb, edge_vertex_map));

    // XXXX
    ChainMap_ chain_map;
    AddChains_(pmb, edge_vertex_map, chain_map);

    // Add faces joining Chain_ points around each original vertex and edge.
    AddVertexFaces_(pmb, chain_map);
    AddEdgeFaces_(pmb, chain_map);

    const PolyMesh result_mesh = pmb.BuildPolyMesh();

    // XXXX
    Debug::Dump3dv dump("/tmp/RMESH.3dv", "Result Beveler2");
    dump.SetLabelFontSize(40);
    dump.AddPolyMesh(result_mesh);

    return result_mesh;
}

void Beveler2::Helper_::BuildSkeletonVertexMap_() {
    const auto &skel_vertices = skeleton_.GetVertices();
    for (size_t i = 0; i < skel_vertices.size(); ++i) {
        const int index = skel_vertices[i].source_index;
        if (index >= 0) {
            ASSERT(static_cast<size_t>(index) < mesh_.vertices.size());
            const auto &pmv = mesh_.vertices[index];
            ASSERT(! Util::MapContains(skel_vertex_map_, pmv));
            skel_vertex_map_[pmv] = i;
        }
    }
    ASSERT(skel_vertex_map_.size() == mesh_.vertices.size());
}

PolyMesh::IndexVec Beveler2::Helper_::AddFaceVertices_(
    const PolyMesh::Face &face, PolyMeshBuilder &pmb,
    PMEdgeMap_ &edge_vertex_map) {
    PolyMesh::IndexVec indices;

    // XXXX Assume no holes for now.
    for (const auto &edge: face.outer_edges) {
        const size_t svi = GetSkeletonVertex_(*edge);

        // Add the new vertex to the offset face.
        const size_t index =
            pmb.AddVertex(Lerp(.2f, edge->v0->point,
                               skeleton_.GetVertices()[svi].point));
        indices.push_back(index);

        // Store the correspondence from the original edge to the new vertex.
        edge_vertex_map[edge] = index;
    }

    // Need to reverse the order to maintain orientation.
    std::reverse(indices.begin(), indices.end());
    return indices;
}

void Beveler2::Helper_::AddChains_(PolyMeshBuilder &pmb,
                                      const PMEdgeMap_ &edge_vertex_map,
                                      ChainMap_ &chain_map) {
    // Use a set to determine which vertices have been processed.
    std::unordered_set<const PolyMesh::Vertex *> processed_vertices;

    for (const auto &edge: mesh_.edges) {
        if (! Util::MapContains(processed_vertices, edge->v0)) {
            processed_vertices.insert(edge->v0);

            // Iterate over all PolyMesh edges incident to this vertex.
            for (const auto &e: PolyMesh::GetVertexEdges(*edge)) {
                const auto &next_edge = e->NextEdgeAroundVertex();

                // Set up and add a Chain_ for each one.
                Chain_ chain;
                chain.end0_index = edge_vertex_map.at(e);
                chain.end1_index = edge_vertex_map.at(&next_edge);

                // XXXX Do this for real.
                chain.interior_index =
                    pmb.AddVertex(Lerp(.45f, pmb.GetVertex(chain.end0_index),
                                       pmb.GetVertex(chain.end1_index)));

                ASSERT(! Util::MapContains(chain_map, e));
                chain_map[e] = chain;
                std::cerr << "XXXX Chain_ for " << e->ToString()
                          << " = " << chain.end0_index
                          << " / " << chain.end1_index
                          << " / " << chain.interior_index << "\n";
            }
        }
    }
}

void Beveler2::Helper_::AddVertexFaces_(PolyMeshBuilder &pmb,
                                           const ChainMap_ &chain_map) {
#if XXXX
    // Use a set to determine which vertices have been processed.
    std::unordered_set<const PolyMesh::Vertex *> processed_vertices;

    for (const auto &edge: mesh_.edges) {
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

void Beveler2::Helper_::AddEdgeFaces_(PolyMeshBuilder &pmb,
                                         const ChainMap_ &chain_map) {
}

size_t Beveler2::Helper_::GetSkeletonVertex_(const PolyMesh::Edge &edge) {
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

void Beveler2::Helper_::DumpSkeleton_() {
    Debug::Dump3dv dump("/tmp/BMESH.3dv", "Beveler2");
    dump.SetLabelFontSize(60);
    dump.SetExtraPrefix("M_");
    dump.AddPolyMesh(mesh_);
    dump.SetExtraPrefix("S_");
    dump.SetLabelOffset(Vector3f(1, 1, .5f));
    dump.AddSkeleton3D(skeleton_);
}

// ----------------------------------------------------------------------------
// Beveler2 functions.
// ----------------------------------------------------------------------------

TriMesh Beveler2::ApplyBevel(const TriMesh &mesh, const Bevel &bevel) {
    // Convert the TriMesh to a PolyMesh and merge coplanar faces.
    PolyMesh poly_mesh(mesh);
    MergeCoplanarFaces(poly_mesh);

    // Use a Helper_ to do the rest.
    Helper_ helper(poly_mesh, bevel);
    return helper.ApplyBevel().ToTriMesh();
}
