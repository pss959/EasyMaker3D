#include "Math/CGALInterface.h"

// Shut up deprecation warnings.
#include <boost/iterator/function_output_iterator.hpp>
#define BOOST_FUNCTION_OUTPUT_ITERATOR_HPP
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/minkowski_sum_3.h>

#include "Assert.h"

// Set this to 1 to enable CNefPolyhedron validation.
#define DO_NEF_CHECK 0

// ----------------------------------------------------------------------------
// CGAL types.
// ----------------------------------------------------------------------------

typedef CGAL::Exact_predicates_exact_constructions_kernel   CKernel;
typedef CGAL::Nef_polyhedron_3<CKernel>                     CNefPolyhedron;
typedef CGAL::Polyhedron_3<CKernel>                         CPolyhedron;
typedef CKernel::Plane_3                                    CPlane3;
typedef CKernel::Point_2                                    CPoint2;
typedef CKernel::Point_3                                    CPoint3;
typedef CPolyhedron::Facet_const_iterator                   CFI;
typedef CPolyhedron::HalfedgeDS                             CHalfedgeDS;
typedef CPolyhedron::HalfedgeDS::Vertex                     CVertex;
typedef CPolyhedron::Halfedge_around_facet_const_circulator CHFC;
typedef CPolyhedron::Vertex_const_iterator                  CVI;

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// The Triangulator class encapsulates triangulation functions.
// ----------------------------------------------------------------------------

class Triangulator_ {
  public:
    // Triangulates a Polygon, possibly with holes.
    std::vector<size_t> Triangulate(const Polygon &poly) {
        // Store a _Vertex instance for each point.
        StoreVertices_(poly);

        // Create a triangulation and add each border to it.
        CCDT triangulation;
        size_t vert_index = 0;
        const std::vector<size_t> &border_counts = poly.GetBorderCounts();
        for (size_t b = 0; b < border_counts.size(); ++b) {
            const size_t count = border_counts[b];
            AddBorderToTriangulation_(vert_index, count, triangulation);
            vert_index += count;
        }

        // Mark facets that are inside the domain bounded by the polygon.
        MarkDomains_(triangulation);

        // Collect triangle indices.
        return GetTriangleIndices_(triangulation);
    }

  private:
    // Stores the location and origin of a vertex.
    struct Vertex_ {
        CPoint2 point;       // Location.
        int     hole_index;  // -1 for outer boundary, >= 0 for hole boundary.
        // Constructor assumes not a hole.
        Vertex_(const Point2f &p) : point(CPoint2(p[0], p[1])),
                                    hole_index(-1) {}
    };

    // Stores info for a face to be able to mark domains.
    struct FaceInfo_ {
        int nesting_level;
        FaceInfo_() { nesting_level = -1; }

        // Returns true if the face is in the domain (after marking).
        bool IsInDomain() const {
            return nesting_level % 2 == 1;  // Nesting level is odd.
        }
    };

    typedef CGAL::Polygon_2<CKernel>                                      CPoly2;
    typedef CGAL::Triangulation_vertex_base_2<CKernel>                    CVB;
    typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo_, CKernel> CFBB;
    typedef CGAL::Constrained_triangulation_face_base_2<CKernel,CFBB>     CFB;
    typedef CGAL::Triangulation_data_structure_2<CVB,CFB>                 CTDS;
    typedef CGAL::Exact_predicates_tag                                    CItag;
    typedef CGAL::Constrained_Delaunay_triangulation_2<CKernel,CTDS,
                                                       CItag>             CCDT;
    typedef CCDT::Vertex_handle                                           CVH;
    typedef CCDT::Face_handle                                             CFH;

    // All vertices of the polygon to triangulate.
    std::vector<Vertex_> vertices_;

    // Maps Vertex_handle (CVH) to index in vector of Vertex_ instances.
    typedef std::unordered_map<CVH, size_t> VMap_;
    VMap_ vmap_;

    // Stores a Vertex_ instance for each point in the given Polygon.
    void StoreVertices_(const Polygon &poly) {
        // Create a Vertex_ for each point.
        const std::vector<Point2f> &points = poly.GetPoints();
        vertices_.reserve(points.size());
        for (const auto &p: points)
            vertices_.push_back(Vertex_(p));
        ASSERT(vertices_.size() == points.size());

        // Update hole indices in each Vertex_ if there are any holes.
        size_t vert_index = 0;
        int    hole_index = -1;
        const std::vector<size_t> &border_counts = poly.GetBorderCounts();
        for (size_t b = 0; b < border_counts.size(); ++b) {
            for (size_t i = 0; i < border_counts[b]; ++i)
                vertices_[vert_index++].hole_index = hole_index;
            ++hole_index;
        }
        ASSERT(vert_index <= points.size());
    }

    void AddBorderToTriangulation_(size_t start_index, size_t count,
                                   CCDT &triangulation) {
        // Store vertex handles so constraint edges can be added.
        std::vector<CVH> vh(count);

        for (size_t i = 0; i < count; ++i) {
            const size_t vert_index = start_index + i;
            vh[i] = triangulation.insert(vertices_[vert_index].point);
            vmap_[vh[i]] = vert_index;
            if (i > 0)
                triangulation.insert_constraint(vh[i - 1], vh[i]);
        }
        // Connect last vertex to first.
        triangulation.insert_constraint(vh[count - 1], vh[0]);
    }

    // Returns the indices for all result triangles in counter-clockwise order.
    std::vector<size_t> GetTriangleIndices_(CCDT &triangulation) {
        std::vector<size_t> indices;
        for (auto face: triangulation.finite_face_handles()) {
            // Skip faces that are not marked as part of the domain.
            if (face->info().IsInDomain()) {
                indices.push_back(vmap_[face->vertex(0)]);
                indices.push_back(vmap_[face->vertex(1)]);
                indices.push_back(vmap_[face->vertex(2)]);
            }
        }
        return indices;
    }

    // Marks domains for a triangulation. Stolen from CGAL doc.
    static void MarkDomains_(CCDT &triangulation) {
        for (CFH f: triangulation.all_face_handles())
            f->info().nesting_level = -1;

        std::list<CCDT::Edge> border;
        MarkDomains_(triangulation, triangulation.infinite_face(), 0, border);
        while (! border.empty()) {
            CCDT::Edge e = border.front();
            border.pop_front();
            CFH n = e.first->neighbor(e.second);
            if (n->info().nesting_level == -1)
                MarkDomains_(triangulation, n,
                             e.first->info().nesting_level + 1, border);
        }
    }
    static void MarkDomains_(CCDT &triangulation, CFH start, int index,
                             std::list<CCDT::Edge> &border) {
        if (start->info().nesting_level != -1)
            return;

        std::list<CFH> queue;
        queue.push_back(start);
        while (! queue.empty()){
            CFH fh = queue.front();
            queue.pop_front();
            if (fh->info().nesting_level == -1) {
                fh->info().nesting_level = index;
                for (int i = 0; i < 3; i++) {
                    CCDT::Edge e(fh, i);
                    CFH n = fh->neighbor(i);
                    if (n->info().nesting_level == -1) {
                        if (triangulation.is_constrained(e))
                            border.push_back(e);
                        else
                            queue.push_back(n);
                    }
                }
            }
        }
    }
};

//! A PolyhedronBuilder_ is used to create a CGAL::Polyhedron from a TriMesh.
class PolyhedronBuilder_ : public CGAL::Modifier_base<CHalfedgeDS> {
  public:
    PolyhedronBuilder_(const TriMesh &mesh) : mesh_(mesh) {}

    void operator()(CHalfedgeDS &hds) {
        // Postcondition: hds is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<CHalfedgeDS> builder(hds, true);
        builder.begin_surface(mesh_.points.size(), mesh_.indices.size() / 3, 0);
        for (const auto &p: mesh_.points)
            builder.add_vertex(CPoint3(p[0], p[1], p[2]));
        for (size_t i = 0; i < mesh_.indices.size(); i += 3) {
            builder.begin_facet();
            builder.add_vertex_to_facet(mesh_.indices[i + 0]);
            builder.add_vertex_to_facet(mesh_.indices[i + 1]);
            builder.add_vertex_to_facet(mesh_.indices[i + 2]);
            builder.end_facet();
        }
        builder.end_surface();
    }

  private:
    const TriMesh mesh_;
};

// ----------------------------------------------------------------------------
// CGAL conversion functions.
// ----------------------------------------------------------------------------

//! Converts a Point3f to a CGAL CPoint3.
static CPoint3 ToCPoint3(const Point3f &p) {
    return CPoint3(p[0], p[1], p[2]);
}

static CPolyhedron BuildPolyhedron_(const TriMesh &mesh) {
    CPolyhedron poly;
    PolyhedronBuilder_ builder(mesh);
    poly.delegate(builder);
    return poly;
}

static CNefPolyhedron BuildNefPolyhedron_(const TriMesh &mesh) {
    CPolyhedron poly = BuildPolyhedron_(mesh);
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

//! Applies a CSG operation.
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

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

std::vector<size_t> TriangulatePolygon(const Polygon &poly) {
    return Triangulator_().Triangulate(poly);
}

MeshValidityCode IsMeshValid(const TriMesh &mesh) {
    CPolyhedron poly = BuildPolyhedron_(mesh);
    if (! poly.is_valid())
        return MeshValidityCode::kInconsistent;
    else if (! poly.is_closed())
        return MeshValidityCode::kNotClosed;
    else if (CGAL::Polygon_mesh_processing::does_self_intersect(poly))
        return MeshValidityCode::kSelfIntersecting;
    else
        return MeshValidityCode::kValid;
}

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
