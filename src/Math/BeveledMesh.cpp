#include "Math/BeveledMesh.h"

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
// Helper_ class. XXXX
// ----------------------------------------------------------------------------

// XXXX
static size_t GetSkelBisectorVertexIndex_(const Skeleton3D &skel,
                                          size_t v0, size_t vn, size_t vp) {
    int vni = static_cast<int>(vn);
    int vpi = static_cast<int>(vp);

    for (const auto &edge: skel.GetEdges()) {
        if ((edge.bisected_index0 == vni && edge.bisected_index1 == vpi) ||
            (edge.bisected_index1 == vni && edge.bisected_index0 == vpi)) {
            if (edge.v0_index == v0)
                return edge.v1_index;
            else if (edge.v1_index == v0)
                return edge.v0_index;
        }
    }
    ASSERTM(false, "No bisector found for " + Util::ToString(v0));
    return 0;
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

    Skeleton3D skel;
    skel.BuildForPolyMesh(poly_mesh);

    Debug::Dump3dv dump("/tmp/BMESH.3dv", "BeveledMesh");
    dump.SetLabelFontSize(60);
    //dump.SetExtraPrefix("T_");
    //dump.SetLabelOffset(Vector3f(1, 1, .1f));
    //dump.AddTriMesh(mesh);
    dump.SetExtraPrefix("M_");
    dump.AddPolyMesh(poly_mesh);
    dump.SetExtraPrefix("S_");
    dump.SetLabelOffset(Vector3f(1, 1, .5f));
    dump.AddSkeleton3D(skel);

    // Maps PolyMesh::Vertex pointer to skeleton vertex index.
    typedef std::unordered_map<const PolyMesh::Vertex *, size_t> SkelVMap_;
    SkelVMap_ skel_vertex_map;
    const auto &skel_vertices = skel.GetVertices();
    for (size_t i = 0; i < skel_vertices.size(); ++i) {
        const auto &sv = skel_vertices[i];
        if (sv.source_index >= 0) {
            ASSERT(static_cast<size_t>(sv.source_index) <
                   poly_mesh.vertices.size());
            const auto &pmv = poly_mesh.vertices[sv.source_index];
            ASSERT(! Util::MapContains(skel_vertex_map, pmv));
            skel_vertex_map[pmv] = i;
        }
    }
    ASSERT(skel_vertex_map.size() == poly_mesh.vertices.size());

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    PolyMeshBuilder pmb;

    // Maps PolyMesh::Edge pointer to the index of the new vertex created for
    // it in the PolyMeshBuilder.
    std::unordered_map<const PolyMesh::Edge *, size_t> edge_vertex_map;

    for (const auto &face: poly_mesh.faces) {
        std::vector<GIndex> face_vert_indices;

        // XXXX Assume no holes for now.
        for (const auto &edge: face->outer_edges) {
            // Get the vertex for the edge.
            const auto &v0 = edge->v0;

            // Get the skeleton vertex at the other end of the bisector
            // starting at v0 for the face.
            // Find the skeleton bisector for the starting vertex of the face.
            const auto &vn = edge->v1;
            const auto &vp = edge->PreviousEdgeInFace().v0;
            const size_t skel_v0 = skel_vertex_map.at(v0);
            const size_t skel_vn = skel_vertex_map.at(vn);
            const size_t skel_vp = skel_vertex_map.at(vp);
            const size_t svi = GetSkelBisectorVertexIndex_(skel, skel_v0,
                                                           skel_vn, skel_vp);
            // Add the new vertex to the offset face.
            const size_t new_vertex_index =
                pmb.AddVertex(Lerp(.2f, v0->point, skel_vertices[svi].point));
            face_vert_indices.push_back(new_vertex_index);

            // Store the correspondence from the original edge to the new
            // vertex.
            edge_vertex_map[edge] = new_vertex_index;
        }
        pmb.AddPolygon(face_vert_indices);
    }

    // Add a face joining all offset points around each original vertex. Use a
    // set to determine which vertices have been processed.
    std::unordered_set<const PolyMesh::Vertex *> processed_vertices;
    for (const auto &edge: poly_mesh.edges) {
        const auto &vertex = edge->v0;
        if (Util::MapContains(processed_vertices, vertex))
            continue;

        processed_vertices.insert(vertex);

        // Add all the offset points for each edge around the vertex.
        pmb.AddPolygon(Util::ConvertVector<GIndex, PolyMesh::Edge *>(
                           PolyMesh::GetVertexEdges(*edge),
                           [&](const PolyMesh::Edge *e){
                               return edge_vertex_map[e]; }));
    }

    const PolyMesh result_poly_mesh = pmb.BuildPolyMesh();

    {
        Debug::Dump3dv dump2("/tmp/RMESH.3dv", "Result BeveledMesh");
        dump2.SetLabelFontSize(30);
        dump2.AddPolyMesh(result_poly_mesh);
    }

    // Convert back to a TriMesh.
    result_mesh_ = poly_mesh.ToTriMesh();
}
