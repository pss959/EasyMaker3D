#include "Math/BeveledMesh.h"

#include <unordered_map>

#include "Math/PolyMesh.h"
// #include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Skeleton3D.h"

#include "Debug/Dump3dv.h" // XXXX
#include "Util/String.h" // XXXX

// ----------------------------------------------------------------------------
// Helper_ class.
// ----------------------------------------------------------------------------

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

#if XXXX
    // Maps vertex index in skeleton to indices of all opposite vertices.
    std::unordered_map<size_t, std::vector<size_t>> edge_map;
    const auto &edges = skel.GetEdges();
    for (size_t i = 0; i < edges.size(); i += 2) {
        edge_map[edges[i + 0]].push_back(edges[i + 1]);
        edge_map[edges[i + 1]].push_back(edges[i + 0]);
    }

    std::cerr << "XXXX ============ Skel edges:\n";
    for (size_t i = 0; i < edges.size(); i += 2)
        std::cerr << "XXXX   From " << edges[i] << " to " << edges[i+1] << "\n";
    std::cerr << "XXXX ============ Skel edge map:\n";
    for (size_t i = 0; i < poly_mesh.vertices.size(); ++i) {
        std::cerr << "XXXX Edges from " << i << " to ["
                  << Util::JoinItems(edge_map[i], ", ") << "]\n";
    }
#endif

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    // PolyMeshBuilder pmb;

    // XXXX Do something!

    // Convert back to a TriMesh.
    result_mesh_ = poly_mesh.ToTriMesh();
}
