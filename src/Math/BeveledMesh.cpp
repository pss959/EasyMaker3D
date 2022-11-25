#include "Math/BeveledMesh.h"

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
// BeveledMesh::Helper_ class. XXXX
// ----------------------------------------------------------------------------

class BeveledMesh::Helper_ {
  public:
    /// The constructor is passed the original PolyMesh to bevel.
    Helper_(const PolyMesh &poly_mesh) : poly_mesh_(poly_mesh) {}

    /// Applies XXXX
    PolyMesh ApplyBevel(const Anglef &max_angle);

  private:
    /// Maps a PolyMesh::Vertex pointer to a skeleton vertex index.
    typedef std::unordered_map<const PolyMesh::Vertex *, size_t> SkelVMap_;

    /// Maps a PolyMesh::Edge pointer to the PolyMeshBuilder index of the new
    /// vertex created for it.
    typedef std::unordered_map<const PolyMesh::Edge *, size_t> PMEdgeMap_;

    const PolyMesh &poly_mesh_;        ///< Original PolyMesh to bevel.
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

    /// Adds a face joining all offset points around each original vertex of
    /// the PolyMesh. The map is used to access the new vertices.
    void AddVertexFaces_(PolyMeshBuilder &pmb,
                         const PMEdgeMap_ &edge_vertex_map);

    /// Returns the index of the skeleton vertex at the other end of the
    /// bisector edge from the starting vertex of the given PolyMesh::Edge
    /// within its face. Asserts if it is not found.
    size_t GetSkeletonVertex_(const PolyMesh::Edge &edge);

    /// XXXX
    void DumpSkeleton_();
};

PolyMesh BeveledMesh::Helper_::ApplyBevel(const Anglef &max_angle) {
    // Build a 3D straight skeleton for the PolyMesh.
    skeleton_.BuildForPolyMesh(poly_mesh_);
    DumpSkeleton_();  // XXXX

    // Map PolyMesh vertices to skeleton vertices.
    BuildSkeletonVertexMap_();

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    PolyMeshBuilder pmb;

    // Maps PolyMesh::Edge pointer to the index of the new vertex created for
    // it in the PolyMeshBuilder.
    PMEdgeMap_ edge_vertex_map;

    // Add vertices for each face of the PolyMesh, adding to edge_vertex_map.
    for (const auto &face: poly_mesh_.faces)
        pmb.AddPolygon(AddFaceVertices_(*face, pmb, edge_vertex_map));

    // Add a face joining all offset points around each original vertex.
    AddVertexFaces_(pmb, edge_vertex_map);

    const PolyMesh result_poly_mesh = pmb.BuildPolyMesh();

    // XXXX
    Debug::Dump3dv dump("/tmp/RMESH.3dv", "Result BeveledMesh");
    dump.SetLabelFontSize(40);
    dump.AddPolyMesh(result_poly_mesh);

    // Convert back to a TriMesh.
    return result_poly_mesh;
}

void BeveledMesh::Helper_::BuildSkeletonVertexMap_() {
    const auto &skel_vertices = skeleton_.GetVertices();
    for (size_t i = 0; i < skel_vertices.size(); ++i) {
        const int index = skel_vertices[i].source_index;
        if (index >= 0) {
            ASSERT(static_cast<size_t>(index) < poly_mesh_.vertices.size());
            const auto &pmv = poly_mesh_.vertices[index];
            ASSERT(! Util::MapContains(skel_vertex_map_, pmv));
            skel_vertex_map_[pmv] = i;
        }
    }
    ASSERT(skel_vertex_map_.size() == poly_mesh_.vertices.size());
}

PolyMesh::IndexVec BeveledMesh::Helper_::AddFaceVertices_(
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

void BeveledMesh::Helper_::AddVertexFaces_(PolyMeshBuilder &pmb,
                                           const PMEdgeMap_ &edge_vertex_map) {
    // Use a set to determine which vertices have been processed.
    std::unordered_set<const PolyMesh::Vertex *> processed_vertices;

    for (const auto &edge: poly_mesh_.edges) {
        const auto &vertex = edge->v0;
        if (! Util::MapContains(processed_vertices, vertex)) {
            processed_vertices.insert(vertex);

            // Add all the offset points for each edge around the vertex.
            pmb.AddPolygon(Util::ConvertVector<GIndex, PolyMesh::Edge *>(
                               PolyMesh::GetVertexEdges(*edge),
                               [&](const PolyMesh::Edge *e){
                                   return edge_vertex_map.at(e); }));
        }
    }
}

size_t BeveledMesh::Helper_::GetSkeletonVertex_(const PolyMesh::Edge &edge) {
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

void BeveledMesh::Helper_::DumpSkeleton_() {
    Debug::Dump3dv dump("/tmp/BMESH.3dv", "BeveledMesh");
    dump.SetLabelFontSize(60);
    dump.SetExtraPrefix("M_");
    dump.AddPolyMesh(poly_mesh_);
    dump.SetExtraPrefix("S_");
    dump.SetLabelOffset(Vector3f(1, 1, .5f));
    dump.AddSkeleton3D(skeleton_);
}

// ----------------------------------------------------------------------------
// BeveledMesh functions.
// ----------------------------------------------------------------------------

BeveledMesh::BeveledMesh(const TriMesh &mesh, const Anglef &max_angle) {
    original_mesh_ = mesh;
    max_angle_     = max_angle;

    // Convert the original TriMesh to a PolyMesh and merge any coplanar faces.
    PolyMesh poly_mesh(mesh);
    MergeCoplanarFaces(poly_mesh);

    // Use a Helper_ to do the beveling.
    Helper_ helper(poly_mesh);
    const PolyMesh result_poly_mesh = helper.ApplyBevel(max_angle);

    result_mesh_ = result_poly_mesh.ToTriMesh();
}
