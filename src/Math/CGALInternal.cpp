#include "Math/CGALInternal.h"

#include "Math/Types.h"

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

CPolyhedron BuildCGALPolyhedron(const TriMesh &mesh) {
    CPolyhedron poly;
    PolyhedronBuilder_ builder(mesh);
    poly.delegate(builder);
    return poly;
}
