#include "Math/MeshValidation.h"

#include "Math/CGALInternal.h"

#define REPAIR_SELF_INTERSECTIONS 0
#define REPORT_SELF_INTERSECTIONS 1

#if REPAIR_SELF_INTERSECTIONS
static void RemoveSelfIntersections_(CPolyhedron &poly) {
    CGAL::Polygon_mesh_processing::experimental::remove_self_intersections(
        poly, CGAL::parameters::preserve_genus(true));
}
#endif

#if REPORT_SELF_INTERSECTIONS
static void ReportSelfIntersections_(CPolyhedron &poly) {
    typedef boost::graph_traits<CPolyhedron>::face_descriptor CFD;
    std::vector<std::pair<CFD, CFD>> intersected_tris;
    CGAL::Polygon_mesh_processing::self_intersections(
        faces(poly), poly, std::back_inserter(intersected_tris));
    std::cerr << intersected_tris.size()
              << " pairs of triangles intersect.\n";
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

MeshValidityCode ValidateTriMesh(const TriMesh &mesh) {
    try {
        CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
        const auto code = IsPolyValid_(poly);
#if REPORT_SELF_INTERSECTIONS
        if (code == MeshValidityCode::kSelfIntersecting)
            ReportSelfIntersections_(poly);
#endif
        return code;
    }
    catch (std::exception &ex) {
        // This is likely an assertion violation. Consider the mesh inconsisent.
        return MeshValidityCode::kInconsistent;
    }
}

MeshValidityCode ValidateAndRepairTriMesh(TriMesh &mesh) {
    try {
        CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
        auto code = IsPolyValid_(poly);
        if (code == MeshValidityCode::kValid)
            return code;

        /// \todo It turns out that the CGAL self-intersection repairing code
        /// throws away lots of vertices and faces, which is bad for this
        /// purpose. Revisit this if CGAL improves polyhedron repairing.
        else if (code == MeshValidityCode::kSelfIntersecting) {
#if REPORT_SELF_INTERSECTIONS
            ReportSelfIntersections_(poly);
#endif
#if REPAIR_SELF_INTERSECTIONS
            // Try to repair and repeat.
            RemoveSelfIntersections_(poly);
            code = IsPolyValid_(poly);
            if (code == MeshValidityCode::kValid)
                mesh = CGALPolyhedronToTriMesh(poly);
#endif
        }

        return code;
    }
    catch (std::exception &ex) {
        // This is likely an assertion violation. Consider the mesh inconsisent.
        return MeshValidityCode::kInconsistent;
    }
}
