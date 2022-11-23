#include "Math/Skeleton2D.h"

#include <unordered_map>

// Skeleton math requires a different kernel.
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

void Skeleton2D::BuildForPolygon(const Polygon &poly) {
    points_.clear();
    distances_.clear();
    edges_.clear();

    // Note that CGAL requires the outer points to be counterclockwise and
    // the holes to be clockwise, which is the same rule used in the Polygon
    // class.

    const auto &border_counts = poly.GetBorderCounts();
    ASSERT(! border_counts.empty());

    // Convert a border to a CIPoly2.
    const auto get_cpoly2 = [](const std::vector<Point2f> &pts){
        CIPoly2 cpoly2;
        for (const auto &p: pts)
            cpoly2.push_back(CIPoint2(p[0], p[1]));
        return cpoly2;
    };

    const CIPoly2 outside = get_cpoly2(poly.GetOuterBorderPoints());
    ASSERT(outside.is_counterclockwise_oriented());

    CISkelPtr skel;
    if (poly.GetHoleCount()) {
        CIPoly2WithHoles cpoly(outside) ;
        for (size_t i = 0; i < poly.GetHoleCount(); ++i) {
            CIPoly2 hole = get_cpoly2(poly.GetHolePoints(i));
            ASSERT(hole.is_clockwise_oriented());
            cpoly.add_hole(hole);
        }
        skel = CGAL::create_interior_straight_skeleton_2(cpoly);
    }
    else {
        skel = CGAL::create_interior_straight_skeleton_2(outside);
    }
    ASSERT(skel);

    // Store results, using a map from vertex ID to index into the points
    // vector.
    std::unordered_map<int, size_t> vertex_map;
    points_.reserve(skel->size_of_vertices());
    distances_.reserve(skel->size_of_vertices());
    for (auto v = skel->vertices_begin(); v != skel->vertices_end(); ++v) {
        vertex_map[v->id()] = points_.size();
        points_.push_back(Point2f(v->point().x(), v->point().y()));
        distances_.push_back(v->time());
    }

    // Add two vertex indices to the edges_ vector for each bisector, using the
    // vertex map. Add bisectors in only one direction (first index < second
    // index).
    edges_.reserve(skel->size_of_halfedges());
    for (auto e = skel->halfedges_begin(); e != skel->halfedges_end(); ++e) {
        if (e->is_bisector() &&
            e->vertex()->id() < e->opposite()->vertex()->id()) {
            edges_.push_back(vertex_map[e->vertex()->id()]);
            edges_.push_back(vertex_map[e->opposite()->vertex()->id()]);
        }
    }
}
