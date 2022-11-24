#include "Math/Skeleton2D.h"

#include <unordered_map>

// Skeleton math requires a special kernel.
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>

#include "Math/CGALInternal.h"
#include "Math/Polygon.h"
#include "Util/Assert.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel    CIKernel;
typedef CIKernel::Point_2                                      CIPoint2;
typedef CGAL::Polygon_2<CIKernel>                              CIPoly2;
typedef CGAL::Polygon_with_holes_2<CIKernel>                   CIPoly2WithHoles;
typedef boost::shared_ptr<CGAL::Straight_skeleton_2<CIKernel>> CISkelPtr;

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Builds and returns a CIPoly2 from a Polygon border (outside or hole). The
/// is_hole flag indicates whether the border is a hole, which determines the
/// order.
static CIPoly2 BorderToCIPoly2_(const std::vector<Point2f> &pts, bool is_hole) {
    CIPoly2 cpoly2;
    for (const auto &p: pts)
        cpoly2.push_back(CIPoint2(p[0], p[1]));

    // Note that CGAL requires the outer points to be counterclockwise and the
    // holes to be clockwise, which is the same rule used in the Polygon class.
    ASSERT(is_hole ? cpoly2.is_clockwise_oriented() :
           cpoly2.is_counterclockwise_oriented());

    return cpoly2;
}

/// Builds and returns a CGAL skeleton from a Polygon.
static CISkelPtr PolygonToSkeleton_(const Polygon &poly) {
    const CIPoly2 outer = BorderToCIPoly2_(poly.GetOuterBorderPoints(), false);

    CISkelPtr skel;
    if (poly.GetHoleCount()) {
        CIPoly2WithHoles cpoly(outer) ;
        for (size_t i = 0; i < poly.GetHoleCount(); ++i)
            cpoly.add_hole(BorderToCIPoly2_(poly.GetHolePoints(i), true));
        skel = CGAL::create_interior_straight_skeleton_2(cpoly);
    }
    else {
        skel = CGAL::create_interior_straight_skeleton_2(outer);
    }
    ASSERT(skel);
    return skel;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Skeleton2D functions.
// ----------------------------------------------------------------------------

void Skeleton2D::BuildForPolygon(const Polygon &poly) {
    const auto skel = PolygonToSkeleton_(poly);

    // Store resulting vertices. If the CGAL vertex id (source index) is
    // greater than the number of source vertices, the vertex was created for
    // the skeleton.
    std::vector<Vertex> vertices;
    vertices.reserve(skel->size_of_vertices());
    const int source_vertex_count = poly.GetPoints().size();
    for (auto v = skel->vertices_begin(); v != skel->vertices_end(); ++v) {
        Vertex vert;
        vert.source_index = v->id() < source_vertex_count ? v->id() : -1;
        vert.point.Set(v->point().x(), v->point().y());
        vert.distance = v->time();
        vertices.push_back(vert);
    }
    SetVertices(vertices);

    // Create a map from CGAL vertex ID to Vertex vector index.
    std::unordered_map<int, size_t> vertex_map;
    size_t v_index = 0;
    for (auto v = skel->vertices_begin(); v != skel->vertices_end(); ++v)
        vertex_map[v->id()] = v_index++;

    // Store resulting bisector edges. Add edges in only one direction (first
    // index < second index).
    std::vector<Edge> edges;
    edges.reserve(skel->size_of_halfedges() / 2);
    for (auto e = skel->halfedges_begin(); e != skel->halfedges_end(); ++e) {
        if (e->is_bisector()) {
            const auto i0 = vertex_map[e->vertex()->id()];
            const auto i1 = vertex_map[e->opposite()->vertex()->id()];
            if (i0 < i1) {
                Edge edge;
                edge.v0_index = i0;
                edge.v1_index = i1;
                edges.push_back(edge);
            }
        }
    }
    SetEdges(edges);
}
