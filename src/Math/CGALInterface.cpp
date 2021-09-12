#include "Math/CGALInterface.h"

// Shut up deprecation warnings.
#include <boost/iterator/function_output_iterator.hpp>
#define BOOST_FUNCTION_OUTPUT_ITERATOR_HPP

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

// ----------------------------------------------------------------------------
// CGAL types.
// ----------------------------------------------------------------------------

typedef CGAL::Exact_predicates_exact_constructions_kernel CKernel;
typedef CGAL::Polyhedron_3<CKernel>                       CPolyhedron;
typedef CGAL::Nef_polyhedron_3<CKernel>                   CNefPolyhedron;
typedef CKernel::Point_3                                  CPoint;
typedef CPolyhedron::HalfedgeDS                           CHalfedgeDS;

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

class PolyhedronBuilder_ : public CGAL::Modifier_base<CHalfedgeDS> {
  public:
    PolyhedronBuilder_(const TriMesh &mesh) : mesh_(mesh) {}

    void operator()(CHalfedgeDS &hds) {
        // Postcondition: hds is a valid polyhedral surface.
        CGAL::Polyhedron_incremental_builder_3<CHalfedgeDS> builder(hds, true);
        builder.begin_surface(mesh_.points.size(), mesh_.indices.size() / 3, 0);
        for (const auto &p: mesh_.points)
            builder.add_vertex(CPoint(p[0], p[1], p[2]));
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
// Helper functions.
// ----------------------------------------------------------------------------

static CPolyhedron BuildPolyhedron_(const TriMesh &mesh) {
    CPolyhedron poly;
    try {
        PolyhedronBuilder_ builder(mesh);
        poly.delegate(builder);
    }
    catch(...) {
        std::cerr << "*** Building CGAL Polyhedron failed\n";
    }
    return poly;
}

#if XXXX
static CNefPolyhedron BuildNefPolyhedron_(const TriMesh &mesh) {
    CPolyhedron poly = BuildPolyhedron_(mesh);
    return CNefPolyhedron(poly);
}
#endif

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

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

