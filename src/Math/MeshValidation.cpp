#include "Math/MeshValidation.h"

#include "Math/CGALInternal.h"
#include "Util/Assert.h"
#include "Util/String.h"

#define REPAIR_SELF_INTERSECTIONS 0
#define REPORT_SELF_INTERSECTIONS 0
#define STITCH_BORDERS            1

#if STITCH_BORDERS
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#endif

#if REPAIR_SELF_INTERSECTIONS || REPORT_SELF_INTERSECTIONS
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#endif

#if REPAIR_SELF_INTERSECTIONS
static void RemoveSelfIntersections_(CPolyhedron &poly) {
    CGAL::Polygon_mesh_processing::experimental::remove_self_intersections(
        poly, CGAL::parameters::preserve_genus(true));
}
#endif

#if REPORT_SELF_INTERSECTIONS
inline std::ostream & operator<<(std::ostream &out, const CPoint3 &p) {
    out << '(' << p.x() << ' ' << p.y() << ' ' << p.z() << ')';
    return out;
}

static void PrintTriFacet_(const TriMesh &mesh, const CPolyhedron::Facet &f) {
    // Store triangle points to find in the TriMesh.
    std::vector<Point3f> tri;
    auto hc = f.facet_begin();
    do {
        const auto &p = hc->vertex()->point();
        tri.push_back(Point3f(CGAL::to_double(p.x().exact()),
                              CGAL::to_double(p.y().exact()),
                              CGAL::to_double(p.z().exact())));
    } while (++hc != f.facet_begin());
    ASSERT(tri.size() == 3U);

    // Find the points in the TriMesh to get vertex indices.
    std::vector<int> indices;
    for (int i = 0; i < 3; ++i) {
        int index = -1;
        for (size_t j = 0; j < mesh.points.size(); ++j) {
            if (mesh.points[j] == tri[i]) {
                index = j;
                break;
            }
        }
        indices.push_back(index);
    }
    std::cerr << " MESH INDICES: " << Util::JoinItems(indices);
}

static void ReportSelfIntersections_(const TriMesh &mesh,
                                     const CPolyhedron &poly) {
    typedef boost::graph_traits<CPolyhedron>::face_descriptor CFD;
    std::vector<std::pair<CFD, CFD>> intersected_tris;
    CGAL::Polygon_mesh_processing::self_intersections(
        faces(poly), poly, std::back_inserter(intersected_tris));
    std::cerr << "*** Intersecting triangles:\n";
    for (const auto &fp: intersected_tris) {
        const auto &f0 = fp.first;
        const auto &f1 = fp.second;
        // For some reason, some facets intersect with themselves. Hmmm.
        if (f0 != f1) {
            std::cerr << "***      ";
            PrintTriFacet_(mesh, *f0);
            std::cerr << " AND";
            PrintTriFacet_(mesh, *f1);
            std::cerr << "\n";
        }
    }
    std::cerr << "*** Mesh: " << mesh.ToString() << "\n";
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
            ReportSelfIntersections_(mesh, poly);
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

#if STITCH_BORDERS
        // See if stitching borders helps. This was needed before CleanMesh()
        // removed duplicate vertices properly.
        CGAL::Polygon_mesh_processing::stitch_borders(poly);
        code = IsPolyValid_(poly);
        if (code == MeshValidityCode::kValid)
            return code;
#endif

        else if (code == MeshValidityCode::kSelfIntersecting) {
#if REPORT_SELF_INTERSECTIONS
            ReportSelfIntersections_(mesh, poly);
#endif
#if REPAIR_SELF_INTERSECTIONS
            /// \todo It turns out that the CGAL self-intersection repairing
            /// code throws away lots of vertices and faces, which is bad for
            /// this purpose. Revisit this if CGAL improves polyhedron
            /// repairing.

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
        // This is likely an assertion violation. Consider the mesh
        // inconsistent.
        return MeshValidityCode::kInconsistent;
    }
}
