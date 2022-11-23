#include "Math/BeveledMesh.h"

#include "Math/PolyMesh.h"
// #include "Math/PolyMeshBuilder.h"
#include "Math/PolyMeshMerging.h"
#include "Math/Skeleton3D.h"

#include "Debug/Dump3dv.h" // XXXX

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
    dump.AddEdges(skel.GetPoints(), skel.GetEdges());

    // XXXX Do something!

    // Create a PolyMeshBuilder to construct the beveled PolyMesh.
    // PolyMeshBuilder pmb;

    // Convert back to a TriMesh.
    result_mesh_ = poly_mesh.ToTriMesh();
}
