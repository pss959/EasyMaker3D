#include "Skeleton.h"

#include <vector>

// Skeleton math requires a different kernel.
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>

#include "Math/CGALInternal.h"
#include "Math/Polygon.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/String.h"  // XXXX

typedef CGAL::Exact_predicates_inexact_constructions_kernel    CIKernel;
typedef CIKernel::Point_2                                      CIPoint2;
typedef CGAL::Polygon_2<CIKernel>                              CIPoly2;
typedef CGAL::Polygon_with_holes_2<CIKernel>                   CIPoly2WithHoles;
typedef boost::shared_ptr<CGAL::Straight_skeleton_2<CIKernel>> CISkelPtr;

template<class K>
static std::string P2S(const CGAL::Point_2<K> &p) {
    return "[" + Util::ToString(p.x()) + " " + Util::ToString(p.y()) + "]";
}

void ComputeSkeleton(const Polygon &poly) {
    const auto &border_counts = poly.GetBorderCounts();
    ASSERT(! border_counts.empty());

    // Note that CGAL requires the outer vertices to be counterclockwise and
    // the holes to be clockwise, which is the same rule used in the Polygon
    // class.
    const auto get_reversed = [](const std::vector<Point2f> &pts){
        CIPoly2 cpoly2;
        for (auto it = pts.rbegin(); it != pts.rend(); ++it) {
            const auto &pt = *it;
            cpoly2.push_back(CIPoint2(pt[0], pt[1]));
        }
        return cpoly2;
    };

    CIPoly2 outside;
    for (const auto &p: poly.GetOuterBorderPoints())
        outside.push_back(CIPoint2(p[0], p[1]));
    ASSERT(outside.is_counterclockwise_oriented());

    CISkelPtr skel;
    if (poly.GetHoleCount()) {
        CIPoly2WithHoles cpoly(outside) ;
        for (size_t i = 0; i < poly.GetHoleCount(); ++i) {
            const CIPoly2 hole = get_reversed(poly.GetHolePoints(i));
            ASSERT(hole.is_clockwise_oriented());
            cpoly.add_hole(hole);
        }
        skel = CGAL::create_interior_straight_skeleton_2(cpoly);
    }
    else {
        skel = CGAL::create_interior_straight_skeleton_2(outside);
    }
    ASSERT(skel);

    const bool do_print = true;  // XXXX poly.GetPoints()[0] == Point2f(-10, -5);
    if (do_print) {
        for (auto e = skel->halfedges_begin(); e != skel->halfedges_end(); ++e) {
            std::cerr << "XXXX "
                      << (e->is_bisector() ? "Bisector" : "Contour")
                      << " from " << P2S(e->opposite()->vertex()->point())
                      << " to " << P2S(e->vertex()->point()) << "\n";
        }
#if XXXX
        for (auto v = skel->vertices_begin(); v != skel->vertices_end(); ++v) {
            std::cerr << "XXXX vertex " << P2S(*v)
                      << " time = " << v->time() << "\n";
        }
#endif
    }
}
