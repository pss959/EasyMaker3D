#include "Math/CGALInternal.h"

#include "Math/TriMesh.h"
#include "Math/Types.h"
#include "Util/Assert.h"

namespace {

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

/// A PolyhedronBuilder_ is used to create a CGAL::Polyhedron from a TriMesh.
class PolyhedronBuilder_ : public CGAL::Modifier_base<CHalfedgeDS> {
  public:
    PolyhedronBuilder_(const TriMesh &mesh) : mesh_(mesh) {}

    void operator()(CHalfedgeDS &hds) {
        // Postcondition: hds is a valid polyhedral surface.
        const bool kVerbose = false;
        CGAL::Polyhedron_incremental_builder_3<CHalfedgeDS> builder(hds,
                                                                    kVerbose);
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

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

CPolyhedron TriMeshToCGALPolyhedron(const TriMesh &mesh) {
    CPolyhedron poly;
    PolyhedronBuilder_ builder(mesh);
    poly.delegate(builder);
    return poly;
}

TriMesh CGALPolyhedronToTriMesh(const CPolyhedron &poly) {
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
