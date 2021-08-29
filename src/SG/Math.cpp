#include "SG/Math.h"

#include <algorithm>
#include <limits>

#include <ion/math/transformutils.h>

#include "Util/String.h"

namespace SG {

// ----------------------------------------------------------------------------
// Plane functions.
// ----------------------------------------------------------------------------

Plane::Plane(const Point3f &point, const Vector3f &norm) {
    normal   = ion::math::Normalized(norm);
    distance = ion::math::Dot(normal, point - Point3f::Zero());
}

std::string Plane::ToString() const {
    return ("PL [n="  + Util::ToString(normal)  +
            " d="     + Util::ToString(distance) +
            "]");
}

// ----------------------------------------------------------------------------
// Ray functions.
// ----------------------------------------------------------------------------

std::string Ray::ToString() const {
    return ("RAY [o="  + Util::ToString(origin)  +
            " d="      + Util::ToString(direction) +
            "]");
}

// ----------------------------------------------------------------------------
// Free functions.
// ----------------------------------------------------------------------------

Ray TransformRay(const Ray &ray, const Matrix4f &m) {
    return Ray(m * ray.origin, m * ray.direction);
}

Bounds TransformBounds(const Bounds &bounds, const Matrix4f &m) {
    Point3f  center  = m * bounds.GetCenter();
    Vector3f extents = .5f * bounds.GetSize();

    // Transform and encapsulate all 8 corners.
    Bounds result;
    for (int i = 0; i < 8; ++i) {
        const Vector3f corner((i & 4) != 0 ? extents[0] : -extents[0],
                              (i & 2) != 0 ? extents[1] : -extents[1],
                              (i & 1) != 0 ? extents[2] : -extents[2]);
        result.ExtendByPoint(center + m * corner);
    }
    return result;
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

bool RayPlaneIntersect(const Ray &ray, const Plane &plane, float &distance) {
    // Use the dot product of the ray direction and the plane normal to
    // detect when the ray is close to parallel to the plane,
    const float dot = ion::math::Dot(ray.direction, plane.normal);
    if (std::abs(dot) < 1e-5f)
        return false;

    // Compute the parametric distance along the ray to the plane.
    const Point3f p = Point3f(plane.distance * plane.normal);
    const float   t = ion::math::Dot(p - ray.origin, plane.normal) / dot;
    if (t < 0.f)
        return false;  // Pointing the wrong way.

    distance = t;
    return true;
}

}  // namespace SG
