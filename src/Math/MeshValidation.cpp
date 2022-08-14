#include "Math/MeshValidation.h"

#include "Math/CGALInternal.h"

static void RemoveSelfIntersections_(CPolyhedron &poly) {
    CGAL::Polygon_mesh_processing::experimental::remove_self_intersections(
        poly, CGAL::parameters::preserve_genus(true));
}

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

MeshValidityCode IsMeshValid(const TriMesh &mesh) {
    try {
        CPolyhedron poly = BuildCGALPolyhedron(mesh);
        auto code = IsPolyValid_(poly);
        if (code == MeshValidityCode::kValid)
            return code;

        else if (code == MeshValidityCode::kSelfIntersecting) {
            RemoveSelfIntersections_(poly);
            code = IsPolyValid_(poly);
        }

        return code;
    }
    catch (std::exception &ex) {
        // This is likely an assertion violation. Consider the mesh inconsisent.
        return MeshValidityCode::kInconsistent;
    }
}
