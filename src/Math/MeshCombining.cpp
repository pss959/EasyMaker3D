#include "Math/MeshCombining.h"

#include "Math/CGALInternal.h"
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/minkowski_sum_3.h>

#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

// Set this to 1 to enable CNefPolyhedron validation.
#define DO_NEF_CHECK 0

namespace {

typedef CGAL::Nef_polyhedron_3<CKernel> CNefPolyhedron;

// ----------------------------------------------------------------------------
// CGAL conversion functions.
// ----------------------------------------------------------------------------

/// Converts a Point3f to a CGAL CPoint3.
static CPoint3 ToCPoint3(const Point3f &p) {
    return CPoint3(p[0], p[1], p[2]);
}

static CNefPolyhedron BuildNefPolyhedron_(const TriMesh &mesh) {
    CPolyhedron poly = BuildCGALPolyhedron(mesh);
    CNefPolyhedron nef_poly(poly);
    ASSERT(nef_poly.is_valid());
    ASSERT(nef_poly.is_simple());
    return nef_poly;
}

static TriMesh ConvertToTriMesh_(const CPolyhedron &poly) {
    TriMesh mesh;

    const size_t vertex_count = poly.size_of_vertices();
    const size_t facet_count  = poly.size_of_facets();
    mesh.points.reserve(vertex_count);
    mesh.indices.reserve(3 * facet_count);

    // Iterate over vertices and output them. While doing this, construct a
    // vertex map to speed up the triangle creation code.
    std::unordered_map<CVI, int> vertex_map(vertex_count);
    int cur_vertex = 0;
    for (CVI v = poly.vertices_begin(); v != poly.vertices_end(); ++v) {
        CPoint3 p = v->point();
        mesh.points.push_back(Point3f(CGAL::to_double(p.x().exact()),
                                      CGAL::to_double(p.y().exact()),
                                      CGAL::to_double(p.z().exact())));
        vertex_map[v] = cur_vertex++;
    }

    // Iterate over facets.
    for (CFI f = poly.facets_begin(); f != poly.facets_end(); ++f) {
        CHFC h = f->facet_begin();
        // Facets must be triangles.
        ASSERT(CGAL::circulator_size(h) == 3);
        do {
            assert(vertex_map.find(h->vertex()) != vertex_map.end());
            mesh.indices.push_back(vertex_map[h->vertex()]);
        } while (++h != f->facet_begin());
    }

    ASSERT(mesh.points.size()  == vertex_count);
    ASSERT(mesh.indices.size() == 3 * facet_count);
    return mesh;
}

#if DO_NEF_CHECK
// This can be used to validate a CNefPolyhedron if errors occur.
static void
CheckNef_(CNefPolyhedron &nef, std::string what) {
    if (! nef.is_valid(false, 1))
        std::cerr << "*** CNefPolyhedron " << what << " is NOT valid\n";
    else if (! nef.is_simple())
        std::cerr << "*** CNefPolyhedron " << what << " is NOT simple\n";
    else
        std::cerr << "*** CNefPolyhedron " << what << " is valid and simple\n";
}
#  define CHECK_NEF_(nef, what) CheckNef_(nef, what)
#else
#  define CHECK_NEF_(nef, what)
#endif

// ----------------------------------------------------------------------------
// Mesh combining functions.
// ----------------------------------------------------------------------------

/// Combines meshes by just concatenating vertices and indices.
static TriMesh Concatenate_(const std::vector<TriMesh> &meshes) {
    const size_t mesh_count = meshes.size();
    ASSERT(mesh_count > 0);

    TriMesh combined;

    // Concatenate points, saving the starting index for each one.
    std::vector<size_t> start_index;
    for (const TriMesh &mesh: meshes) {
        start_index.push_back(combined.points.size());
        Util::AppendVector(mesh.points, combined.points);
    }

    // Concatenate indices, adjusting by the shift in vertex position.
    for (size_t i = 0; i < meshes.size(); ++i) {
        const TriMesh &mesh = meshes[i];
        const size_t start = start_index[i];
        auto offset = [start](GIndex index){ return start + index; };
        Util::AppendVector(
            Util::ConvertVector<GIndex, GIndex>(mesh.indices, offset),
            combined.indices);
    }

    return combined;
}

/// Applies a CSG operation.
static TriMesh ApplyCSG_(const std::vector<TriMesh> &meshes,
                         MeshCombiningOperation operation) {
    const size_t mesh_count = meshes.size();
    ASSERT(mesh_count >= 2);

    std::vector<CPolyhedron> polys;
    polys.reserve(mesh_count);

    // Create a CPolyhedron for the first mesh.
    CPolyhedron poly = BuildCGALPolyhedron(meshes[0]);

    // Apply the operation to a CPolyhedron for the remaining meshes.
    for (size_t i = 1; i < mesh_count; ++i) {
        CPolyhedron poly2 = BuildCGALPolyhedron(meshes[i]);
        if (operation == MeshCombiningOperation::kCSGUnion)
            CGAL::Polygon_mesh_processing::corefine_and_compute_union(
                poly, poly2, poly);
        else if (operation == MeshCombiningOperation::kCSGIntersection)
            CGAL::Polygon_mesh_processing::corefine_and_compute_intersection(
                poly, poly2, poly);
        else  // kCSGDifference.
            CGAL::Polygon_mesh_processing::corefine_and_compute_difference(
                poly, poly2, poly);
    }

    // Convert the result back to a TriMesh.
    return ConvertToTriMesh_(poly);
}

// Applies a convex hull operation.
static TriMesh ApplyConvexHull_(const std::vector<TriMesh> &meshes) {
    const size_t mesh_count = meshes.size();
    ASSERT(mesh_count > 0);

    // Count all vertices in all meshes.
    auto counter =
        [](int sum, const TriMesh &tm){ return sum + tm.points.size(); };
    const size_t vertex_count =
        std::accumulate(meshes.begin(), meshes.end(), 0, counter);

    // Gather vertices from all meshes into one vector.
    std::vector<CPoint3> points;
    points.reserve(vertex_count);
    for (size_t i = 0; i < mesh_count; ++i) {
        for (size_t j = 0; j < meshes[i].points.size(); ++j)
            points.push_back(ToCPoint3(meshes[i].points[j]));
    }
    CPolyhedron result;
    CGAL::convex_hull_3(points.begin(), points.end(), result);
    return ConvertToTriMesh_(result);
}

// Applies a Minkowski sum operation.
static TriMesh ApplyMinkowskiSum_(const std::vector<TriMesh> &meshes) {
    const size_t mesh_count = meshes.size();
    ASSERT(mesh_count == 2);

    // Create a CNefPolyhedron for each mesh.
    std::vector<CNefPolyhedron> nefs;
    nefs.reserve(2);
    CNefPolyhedron nef0 = BuildNefPolyhedron_(meshes[0]);
    CNefPolyhedron nef1 = BuildNefPolyhedron_(meshes[1]);
    CHECK_NEF_(nef0, "Minkowski Polyhedron 0");
    CHECK_NEF_(nef1, "Minkowski Polyhedron 1");

    // Apply the sum operation.
    CNefPolyhedron result = CGAL::minkowski_sum_3(nef0, nef1);
    CHECK_NEF_(result, "Result Minkowski Polyhedron");

    // Convert the result back to a Polyhedron and then to a TriMesh.
    CPolyhedron poly;
    result.convert_to_polyhedron(poly);
    return ConvertToTriMesh_(poly);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

TriMesh CombineMeshes(const std::vector<TriMesh> &meshes,
                      MeshCombiningOperation operation) {
    switch (operation) {
      case MeshCombiningOperation::kConcatenate:
        return Concatenate_(meshes);
      case MeshCombiningOperation::kCSGUnion:
      case MeshCombiningOperation::kCSGIntersection:
      case MeshCombiningOperation::kCSGDifference:
        return ApplyCSG_(meshes, operation);
      case MeshCombiningOperation::kConvexHull:
        return ApplyConvexHull_(meshes);
      case MeshCombiningOperation::kMinkowskiSum:
        return ApplyMinkowskiSum_(meshes);
    }
    // Should never get here.
    ASSERT(false);
    return TriMesh();
}

TriMesh ClipMesh(const TriMesh &mesh, const Plane &plane) {
    CPolyhedron poly = BuildCGALPolyhedron(mesh);

    // Clip it by the plane.
    const Vector4f coeffs = plane.GetCoefficients();

    CGAL::Polygon_mesh_processing::clip(
        poly, CPlane3(coeffs[0], coeffs[1], coeffs[2], coeffs[3]),
        CGAL::parameters::clip_volume(true));

    return ConvertToTriMesh_(poly);
}
