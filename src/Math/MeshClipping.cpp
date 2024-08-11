//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/MeshClipping.h"

#include <algorithm>
#include <limits>

#include "Math/CGALInternal.h"
#include "Math/Linear.h"
#include "Math/Plane.h"
#include "Math/TriMesh.h"
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
