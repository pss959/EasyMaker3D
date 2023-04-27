#include "Math/MeshDividing.h"

#include "Math/CGALInternal.h"
#include "Util/KLog.h"

#include <CGAL/Polygon_mesh_processing/clip.h>

namespace {

// ----------------------------------------------------------------------------
// CGAL conversion functions.
// ----------------------------------------------------------------------------

/// Converts a Plane to a CGAL CPlane3.
static CPlane3 ToCPlane3(const Plane &plane) {
    const Vector4f coeffs = plane.GetCoefficients();
    return CPlane3(coeffs[0], coeffs[1], coeffs[2], coeffs[3]);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

TriMesh ClipMesh(const TriMesh &mesh, const Plane &plane) {
    KLOG('C', "Clipping TriMesh with " << plane);

    CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
    CGAL::Polygon_mesh_processing::clip(poly, ToCPlane3(plane),
                                        CGAL::parameters::clip_volume(true));
    return CGALPolyhedronToTriMesh(poly);
}

TriMesh SplitMesh(const TriMesh &mesh, const Plane &plane) {
    KLOG('C', "Splitting TriMesh with " << plane);

    CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
    CGAL::Polygon_mesh_processing::split(poly, ToCPlane3(plane));
    return CGALPolyhedronToTriMesh(poly);
}
