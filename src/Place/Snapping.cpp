#include "Place/Snapping.h"

#include <cmath>

#include "Util/Assert.h"

Dimensionality SnapToBounds(const Bounds &bounds, Point3f &p,
                            const Vector3f &tolerance) {
    Dimensionality snapped_dims;

    auto check_val = [&](const Point3f &bounds_pt, int dim, float &min_dist){
        ASSERT(dim >= 0 && dim <= 2);
        const float dist = std::fabs(p[dim] - bounds_pt[dim]);
        if (dist <= tolerance[dim] && dist < min_dist) {
            p[dim]   = bounds_pt[dim];
            min_dist = dist;
        }
    };

    // Test for p being close to the min, max, or center value in all 3
    // dimensions and snap to it if so.
    for (int dim = 0; dim < 3; ++dim) {
        float min_dist = 100000 * tolerance[dim];
        check_val(bounds.GetMinPoint(), dim, min_dist);
        check_val(bounds.GetCenter(),   dim, min_dist);
        check_val(bounds.GetMaxPoint(), dim, min_dist);
        if (min_dist < tolerance[dim])
            snapped_dims.AddDimension(dim);
    }

    return snapped_dims;
}

