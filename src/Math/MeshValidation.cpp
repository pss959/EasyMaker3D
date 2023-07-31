#include "Math/MeshValidation.h"

#include "Math/CGALInternal.h"
#include "Util/Assert.h"
#include "Util/String.h"

#define REPAIR_SELF_INTERSECTIONS 0
#define REPORT_SELF_INTERSECTIONS 0
#define REPORT_BORDER_EDGES       0  // Useful when is_closed() returns false.
#define STITCH_BORDERS            1

#if STITCH_BORDERS
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#endif

#if REPAIR_SELF_INTERSECTIONS || REPORT_SELF_INTERSECTIONS
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#endif

// ----------------------------------------------------------------------------
// Helper classes and functions.
// ----------------------------------------------------------------------------

// Maps vertices in a CPolyhedron to TriMesh indices.
class VertexMap_ {
  public:
    explicit VertexMap_(const TriMesh &mesh) : mesh_(mesh) {}

    GIndex FindPoint(const CPoint3 &p) {
        const Point3f pt(CGAL::to_double(p.x().exact()),
                         CGAL::to_double(p.y().exact()),
                         CGAL::to_double(p.z().exact()));
        // Linear search. Efficiency does not matter here.
        GIndex index = -1;
        for (size_t i = 0; i < mesh_.points.size(); ++i) {
            if (mesh_.points[i] == pt) {
                index = i;
                break;
            }
        }
        return index;
    }

  private:
    const TriMesh &mesh_;
};

static std::vector<GIndex> GetBorderEdges_(const TriMesh &mesh,
                                           CPolyhedron poly) {
    std::vector<GIndex> indices;
    poly.normalize_border();
    VertexMap_ vmap(mesh);
    for (auto it = poly.border_halfedges_begin();
         it != poly.halfedges_end(); ++it) {
        const auto &p0 = it->vertex()->point();
        const auto &p1 = it->next()->vertex()->point();
        indices.push_back(vmap.FindPoint(p0));
        indices.push_back(vmap.FindPoint(p1));
    }
    return indices;
}

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
    std::cerr << " MESH INDICES:";

    VertexMap_ vmap(mesh);
    auto hc = f.facet_begin();
    do {
        const auto &p = hc->vertex()->point();
        std::cerr << " " << vmap.FindPoint(hc->vertex()->point()) << "\n";
    } while (++hc != f.facet_begin());
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

#if REPORT_BORDER_EDGES
static void ReportBorderEdges_(const TriMesh &mesh, CPolyhedron poly) {
    const auto indices = GetBorderEdges_(mesh, poly);
    for (size_t i = 0; i < indices.size(); i += 2) {
        std::cerr << "*** Border Edge from V" << indices[i]
                  << " to V" << indices[i + 1] << "\n";
    }
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
#if REPORT_BORDER_EDGES
        if (code == MeshValidityCode::kNotClosed)
            ReportBorderEdges_(mesh, poly);
#endif
        return code;
    }
    catch (std::exception &ex) {
        // This is likely an assertion violation. Consider the mesh
        // inconsistent.
        return MeshValidityCode::kInconsistent;
    }
}

MeshValidityCode ValidateAndRepairTriMesh(TriMesh &mesh) {
    try {
        CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
        auto code = IsPolyValid_(poly);
        if (code == MeshValidityCode::kValid)
            return code;

        else if (code == MeshValidityCode::kNotClosed) {
#if STITCH_BORDERS
            // See if stitching borders helps. This was needed before
            // CleanMesh() removed duplicate vertices properly.
            CGAL::Polygon_mesh_processing::stitch_borders(poly);
            code = IsPolyValid_(poly);
            if (code == MeshValidityCode::kValid)
                return code;
#endif
#if REPORT_BORDER_EDGES
            ReportBorderEdges_(mesh, poly);
#endif
        }

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

std::vector<GIndex> GetBorderEdges(const TriMesh &mesh) {
    CPolyhedron poly = TriMeshToCGALPolyhedron(mesh);
    return GetBorderEdges_(mesh, poly);
}
