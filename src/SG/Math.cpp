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

Plane::Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2) {
    normal   = ion::math::Normalized(ion::math::Cross(p1 - p0, p2 - p0));
    distance = ion::math::Dot(normal, Vector3f(p0));
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
// Transformation functions.
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

// ----------------------------------------------------------------------------
// Intersection functions.
// ----------------------------------------------------------------------------

bool RayBoundsIntersect(const Ray &ray, const Bounds &bounds, float &distance) {
    Bounds::Face face;
    bool         is_entry;
    return RayBoundsIntersectFace(ray, bounds, distance, face, is_entry) &&
        is_entry;
}

bool RayBoundsIntersectFace(const Ray &ray, const Bounds &bounds,
                            float &distance, Bounds::Face &face,
                            bool &is_entry) {
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
    // If we get here, there is at least one intersection. Make sure one is
    // within the ray bounds.
    if (t_near > 0.f) {
        distance = t_near;
        face     = Bounds::GetFace(face_dim, face_max);
        is_entry = true;
        return true;
    }
    else {
        distance = t_far;
        face     = Bounds::GetFace(face_dim, ! face_max);  // Opposite side.
        is_entry = false;
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

bool RayTriangleIntersect(const Ray &ray, const Point3f &p0,
                          const Point3f &p1, const Point3f &p2,
                          float &distance, Vector3f &barycentric) {
    using ion::math::WithoutDimension;

    // Intersect the plane containing the three points.
    const Plane plane(p0, p1, p2);
    float distance_to_plane;
    if (! RayPlaneIntersect(ray, plane, distance_to_plane))
        return false;

    // Reduce the rest of the intersection computation to two dimensions by
    // projecting everything onto one of the principal coordinate planes.  Use
    // the component of the plane normal with the largest magnitude to find the
    // indices of the best plane to use.
    const int max_dim = GetMaxAbsElementIndex(plane.normal);
    const Point2f p0_in_plane = WithoutDimension(p0, max_dim);
    const Point2f p1_in_plane = WithoutDimension(p1, max_dim);
    const Point2f p2_in_plane = WithoutDimension(p2, max_dim);
    const Point3f inter       = ray.origin + distance_to_plane * ray.direction;
    const Point2f inter_in_plane = WithoutDimension(inter, max_dim);

    // Compute barycentric coordinates of the point with respect to the
    // triangle. If they indicate that the point is outside the triangle,
    // return false.
    Vector3f bary;
    if (! ComputeBarycentric(inter_in_plane, p0_in_plane, p1_in_plane,
                             p2_in_plane, bary))
        return false;

    distance    = distance_to_plane;
    barycentric = bary;
    return true;
}

bool RayTriMeshIntersect(const Ray &ray, const TriMesh &mesh,
                         float &distance, TriMesh::Hit &hit) {
    float min_distance = std::numeric_limits<float>::max();
    bool hit_any = false;

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        const int      i0 = mesh.indices[i];
        const int      i1 = mesh.indices[i + 1];
        const int      i2 = mesh.indices[i + 2];
        const Point3f &p0 = mesh.points[i0];
        const Point3f &p1 = mesh.points[i1];
        const Point3f &p2 = mesh.points[i2];

        // Skip this triangle if there is no intersection or it is past the
        // closest one found so far.
        float    tdistance;
        Vector3f barycentric;
        if (! RayTriangleIntersect(ray, p0, p1, p2, tdistance, barycentric) ||
            tdistance >= min_distance)
            continue;

        // Update the return info with the triangle.
        distance   = tdistance;
        hit.point  = ray.origin + tdistance * ray.direction;
        hit.normal = ComputeNormal(p0, p1, p2);
        hit.indices.Set(i0, i1, i2);
        hit.barycentric = barycentric;

        min_distance = tdistance;
        hit_any = true;
    }
    return hit_any;
}

// ----------------------------------------------------------------------------
// General linear algebra functions.
// ----------------------------------------------------------------------------

bool AreClose(float a, float b, float tolerance) {
    return std::abs(a -b) <= tolerance;
}

bool AreClose(const Vector3f &a, const Vector3f &b, float tolerance) {
    return ion::math::LengthSquared(b - a) <= tolerance;
}

int GetMinElementIndex(const Vector3f &v) {
    int   min_index = 0;
    float min_value = v[0];
    if (v[1] < min_value) {
        min_index = 1;
        min_value = v[1];
    }
    if (v[2] < min_value)
        min_index = 2;
    return min_index;
}

int GetMaxElementIndex(const Vector3f &v) {
    int   max_index = 0;
    float max_value = v[0];
    if (v[1] > max_value) {
        max_index = 1;
        max_value = v[1];
    }
    if (v[2] > max_value)
        max_index = 2;
    return max_index;
}

int GetMinAbsElementIndex(const Vector3f &v) {
    return GetMinElementIndex(Vector3f(std::abs(v[0]),
                                       std::abs(v[1]),
                                       std::abs(v[2])));
}

int GetMaxAbsElementIndex(const Vector3f &v) {
    return GetMaxElementIndex(Vector3f(std::abs(v[0]),
                                       std::abs(v[1]),
                                       std::abs(v[2])));
}

Vector3f ComputeNormal(const Point3f &p0, const Point3f &p1,
                       const Point3f &p2) {
    return ion::math::Normalized(ion::math::Cross(p1 - p0, p2 - p0));
}

bool ComputeBarycentric(const Point2f &p, const Point2f & a,
                        const Point2f &b, const Point2f &c, Vector3f &bary) {
    using ion::math::Dot;

    const Vector2f v0 = b - a;
    const Vector2f v1 = c - a;
    const Vector2f v2 = p - a;

    const float d00 = Dot(v0, v0);
    const float d01 = Dot(v0, v1);
    const float d11 = Dot(v1, v1);
    const float d20 = Dot(v2, v0);
    const float d21 = Dot(v2, v1);

    const float denom = d00 * d11 - d01 * d01;
    if (AreClose(denom, 0.f))  // Should never happen.
        return false;

    const float alpha = (d11 * d20 - d01 * d21) / denom;
    const float beta  = (d00 * d21 - d01 * d20) / denom;
    if (alpha < 0.f || beta < 0.f || alpha + beta > 1.f)
        return false;

    bary.Set(1.f - alpha - beta, alpha, beta);
    return true;
}

}  // namespace SG
