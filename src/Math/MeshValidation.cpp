#include "Math/MeshValidation.h"

#include "Math/CGALInternal.h"

MeshValidityCode IsMeshValid(const TriMesh &mesh) {
    try {
        CPolyhedron poly = BuildCGALPolyhedron(mesh);
        if (! poly.is_valid())
            return MeshValidityCode::kInconsistent;
        else if (! poly.is_closed())
            return MeshValidityCode::kNotClosed;
        else if (CGAL::Polygon_mesh_processing::does_self_intersect(poly))
            return MeshValidityCode::kSelfIntersecting;
        else
            return MeshValidityCode::kValid;
    }
    catch (std::exception &ex) {
        // This is likely an assertion violation. Consider the mesh inconsisent.
        return MeshValidityCode::kInconsistent;
    }
}
