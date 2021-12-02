#include "Math/MeshCombining.h"

#include "Math/CGALInternal.h"

#include "Util/Assert.h"

// Set this to 1 to enable CNefPolyhedron validation.
#define DO_NEF_CHECK 0

namespace {

// ----------------------------------------------------------------------------
// CGAL conversion functions.
// ----------------------------------------------------------------------------

/// Converts a Point3f to a CGAL CPoint3.
static CPoint3 ToCPoint3(const Point3f &p) {
    return CPoint3(p[0], p[1], p[2]);
}

static CNefPolyhedron BuildNefPolyhedron_(const TriMesh &mesh) {
    CPolyhedron poly = BuildCGALPolyhedron(mesh);
    return CNefPolyhedron(poly);
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
    if (! nef.is_valid(true, 1))
        std::cerr << "*** CNefPolyhedron " << what << " is NOT valid\n";
    else
        std::cerr << "*** CNefPolyhedron " << what << " is valid\n";
}
#  define CHECK_NEF_(nef, what) CheckNef_(nef, what)
#else
#  define CHECK_NEF_(nef, what)
#endif

// ----------------------------------------------------------------------------
// Mesh combining functions.
// ----------------------------------------------------------------------------

/// Applies a CSG operation.
static TriMesh ApplyCSG_(const std::vector<TriMesh> &meshes,
                         MeshCombiningOperation operation) {
    const size_t mesh_count = meshes.size();
    ASSERT(mesh_count >= 2);

    std::vector<CNefPolyhedron> nefs;
    nefs.reserve(mesh_count);

    // Create a CNefPolyhedron for the first mesh.
    CNefPolyhedron nef = BuildNefPolyhedron_(meshes[0]);
    CHECK_NEF_(nef, "Polyhedron 0");

    // Apply the operation to a CNefPolyhedron for the remaining meshes.
    for (size_t i = 1; i < mesh_count; ++i) {
        CNefPolyhedron nef2 = BuildNefPolyhedron_(meshes[i]);
        CHECK_NEF_(nef, "Polyhedron " + std::to_string(i));

        if (operation == MeshCombiningOperation::kCSGUnion)
            nef += nef2;
        else if (operation == MeshCombiningOperation::kCSGIntersection)
            nef *= nef2;
        else  // kCSGDifference.
            nef -= nef2;
        CHECK_NEF_(nef, "Result Polyhedron " + std::to_string(i));
    }

    // Convert the result back to a Polyhedron and then to a TriMesh.
    CPolyhedron poly;
    nef.convert_to_polyhedron(poly);
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
