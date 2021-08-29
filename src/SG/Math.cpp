#include "SG/Math.h"

#include <algorithm>
#include <limits>

#include <ion/math/transformutils.h>

namespace SG {

Ray TransformRay(const Ray &ray, const Matrix4f &m) {
    return Ray(m * ray.origin, m * ray.direction);
}

bool RayBoundsIntersect(const Ray &ray, const Bounds &bounds, float &distance) {
    Bounds::Face face;
    return RayBoundsIntersectFace(ray, bounds, distance, face);
}

bool RayBoundsIntersectFace(const Ray &ray, const Bounds &bounds,
                            float &distance, Bounds::Face &face) {
    // Use Kay/Kajiya/Haines "slabs" method.
    float t_near = -std::numeric_limits<float>::max();
    float t_far  =  std::numeric_limits<float>::max();

    // These are used to select the Face.
    int  face_dim;
    bool face_max;

    for (int dim = 0; dim < 3; ++dim) {
        float p     = ray.origin[dim];
        float d     = ray.direction[dim];
        float b_min = bounds.GetMinPoint()[dim];
        float b_max = bounds.GetMaxPoint()[dim];

        // Check if ray is almost parallel to the plane perpendicular to the
        // axis in this dimension.
        if (std::abs(d) < 1e-4) {
            // In the parallel case, the ray must lie between the sides of the
            // box in that dimension.
            if (p < b_min || p > b_max)
                return false;
        } else {
            // In the non-parallel case, find the parametric values at the
            // intersection points with the two planes.
            const float t0 = (b_min - p) / d;
            const float t1 = (b_max - p) / d;
            const float t_min = std::min(t0, t1);
            const float t_max = std::max(t0, t1);
            if (t_min > t_near) {
                t_near = t_min;
                face_dim = dim;
                face_max = t1 == t_min;
            }
            if (t_max < t_far)
                t_far = t_max;
            // Check if the ray misses the box in this dimension.
            if (t_near > t_far || t_far < 0.f)
                return false;
        }
    }
    // If we get here, there is an intersection. Make sure it is within
    // the ray bounds.
    if (t_near > 0.f) {
        distance = t_near;
        face     = Bounds::GetFace(face_dim, face_max);
        return true;
    }
    return false;
}

}  // namespace SG
