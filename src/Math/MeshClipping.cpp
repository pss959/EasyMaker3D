#include "Math/MeshClipping.h"

#include <algorithm>
#include <limits>

#include "Math/CGALInternal.h"
#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

#include <CGAL/Polygon_mesh_processing/clip.h>

TriMesh ClipMesh(const TriMesh &mesh, const Plane &plane) {
    KLOG('C', "Clipping TriMesh with " << plane);

    CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);

    const Vector4f coeffs = plane.GetCoefficients();
    const CPlane3 cplane(coeffs[0], coeffs[1], coeffs[2], coeffs[3]);

    CGAL::Polygon_mesh_processing::clip(poly, cplane,
                                        CGAL::parameters::clip_volume(true));

    return CGALPolyhedronToTriMesh(poly);
}
