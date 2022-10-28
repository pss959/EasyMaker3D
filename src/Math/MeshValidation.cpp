#include "Math/MeshValidation.h"

#include "Math/CGALInternal.h"

#define REPAIR_SELF_INTERSECTIONS 0

#if REPAIR_SELF_INTERSECTIONS
static void RemoveSelfIntersections_(CPolyhedron &poly) {
    CGAL::Polygon_mesh_processing::experimental::remove_self_intersections(
        poly, CGAL::parameters::preserve_genus(true));
}
#endif

static MeshValidityCode IsPolyValid_(const CPolyhedron &poly) {
    if (! poly.is_valid())
        return MeshValidityCode::kInconsistent;
    else if (! poly.is_closed())
        return MeshValidityCode::kNotClosed;
    else if (CGAL::Polygon_mesh_processing::does_self_intersect(poly))
        return MeshValidityCode::kSelfIntersecting;
    else
        return MeshValidityCode::kValid;
}

MeshValidityCode ValidateTriMesh(TriMesh &mesh) {
    try {
        CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
        auto code = IsPolyValid_(poly);
        if (code == MeshValidityCode::kValid)
            return code;

        /// \todo It turns out that the CGAL self-intersection repairing code
        /// throws away lots of vertices and faces, which is bad for this
        /// purpose. Revisit this if CGAL improves polyhedron repairing.
#if REPAIR_SELF_INTERSECTIONS
        else if (code == MeshValidityCode::kSelfIntersecting) {
            // Try to repair and repeat.
            RemoveSelfIntersections_(poly);
            code = IsPolyValid_(poly);
            if (code == MeshValidityCode::kValid)
                mesh = CGALPolyhedronToTriMesh(poly);
        }
#endif

        return code;
    }
    catch (std::exception &ex) {
        // This is likely an assertion violation. Consider the mesh inconsisent.
        return MeshValidityCode::kInconsistent;
    }
}
