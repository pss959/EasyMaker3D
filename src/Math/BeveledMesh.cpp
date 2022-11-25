#include "Math/BeveledMesh.h"

#include <unordered_map>

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

#if XXXX
    // Maps vertex index in skeleton to indices of all opposite vertices.
    std::unordered_map<size_t, std::vector<size_t>> skel_edge_map;
    for (const auto &edge: skel.GetEdges()) {
        skel_edge_map[edge.v0_index].push_back(edge.v1_index);
        skel_edge_map[edge.v1_index].push_back(edge.v0_index);
    }

    // Maps PolyMesh::Vertex pointer to all skeleton edges indident to it.
    std::unordered_map<const PolyMesh::Vertex *, std::vector<size_t>> skel_edge_map;
    for (const auto &edge: skel.GetEdges()) {
        skel_edge_map[edge.v0_index].push_back(edge.v1_index);
        skel_edge_map[edge.v1_index].push_back(edge.v0_index);
    }
#endif

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    PolyMeshBuilder pmb;

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
            std::cerr << "XXXX Found " << svi << " for "
                      << vp->id << "/" << v0->id << "/" << vn->id
                      << " ("
                      << skel_vp << "/" << skel_v0 << "/" << skel_vn
                      << ") in " << face->id << "\n";


#if XXXX
            // Get the skeleton vertex at the other end of the bisector
            // starting at v0 for the face.
            const std::string key = face->id + "/" + v0->id;
            const auto &sv1 = skel_bisector_map[key];
#endif

#if XXXX
            // Find the skeleton bisector for the starting vertex of the face.
            const auto &vn = edge->v1;
            const auto &vp = edge->PreviousEdgeInFace().v0;

            const size_t skel_v0 = skel_vertex_map.at(v0);
            const size_t skel_vn = skel_vertex_map.at(vn);
            const size_t skel_vp = skel_vertex_map.at(vp);
            if (edge->id == "E29") // XXXX
            std::cerr << "XXXX Edge " << edge->id << " in " << face->id
                      << ": v0=" << v0->id
                      << " vp=" << vp->id
                      << " vn=" << vn->id
                      << " sv0=" << skel_v0
                      << " svn=" << skel_vn
                      << " svp=" << skel_vp
                      << " edges="
                      << Util::JoinItems(skel_edge_map.at(skel_v0))
                      << "\n";
            //for (const auto &other_v: skel_edge_map.at(skel_v0)) {
            //}
#endif
            
            face_vert_indices.push_back(
                pmb.AddVertex(Lerp(.2f, v0->point, skel_vertices[svi].point)));
        }
        pmb.AddPolygon(face_vert_indices);
    }

    const PolyMesh result_poly_mesh = pmb.BuildPolyMesh();
    dump.SetExtraPrefix("R_");
    dump.SetLabelOffset(Vector3f(1, 1.5f, .75f));
    dump.AddPolyMesh(result_poly_mesh);

    // XXXX Do something!

    // Convert back to a TriMesh.
    result_mesh_ = poly_mesh.ToTriMesh();
}
