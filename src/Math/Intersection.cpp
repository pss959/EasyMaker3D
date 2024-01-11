#include "Math/Intersection.h"

#include <algorithm>
#include <limits>

#include <ion/math/angleutils.h>
#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/Plane.h"
#include "Math/Ray.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Solves a quadratic equation with coefficients \p a, \p b, \p c. If there
/// are 2 positive real roots, this sets \p root0 to the smaller, sets \p root1
/// to the larger, and returns true. If there is 1 positive real root, this
/// sets both \p root0 and \p root1 to it and returns true. Otherwise, it just
/// returns false.
static bool SolveQuadratic_(float a, float b, float c,
                            float &root0, float &root1) {
    // If the discriminant is not positive, there are no positive real roots.
    const float discriminant = b * b - 4 * a * c;
    if (discriminant <= 0)
	return 0;

    // Compute t0 as (-b - sqrt(b^2 - 4ac)) / 2a
    //    and  t1 as (-b + sqrt(b^2 - 4ac)) / 2a
    //
    const float sqroot = std::sqrt(discriminant);
    const float denom  = 1.f / (2 * a);
    const float t0 = (-b - sqroot) * denom;
    const float t1 = (-b + sqroot) * denom;

    const float smaller = std::min(t0, t1);
    const float larger  = std::max(t0, t1);
    if (smaller > 0) {  // There are 2 positive real roots.
        root0 = smaller;
        root1 = larger;
        return 2;
    }
    else if (larger > 0) { // There is 1 positive real root.
        root0 = root1 = larger;
        return 1;
    }
    return 0;  // No positive roots.
}

/// Solves a quadratic equation with coefficients \p a, \p b, \p c. If there
/// are any positive real roots, this sets \p solution to the smaller one and
/// returns true. Otherwise, it just returns false.
static bool SolveQuadratic_(float a, float b, float c, float &solution) {
    float root0, root1;
    if (SolveQuadratic_(a, b, c, root0, root1)) {
        solution = root0;
        return true;
    }
    else {
        return false;
    }
}

// ----------------------------------------------------------------------------
// Public functions.
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
    int  face_dim = -1;
    bool face_max = false;

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
        }
        else {
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
    // If there was somehow no intersection.
    if (face_dim < 0) {
        return false;
    }
    else if (t_near > 0.f) {
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
    // Intersect the plane containing the three points.
    const Plane plane(p0, p1, p2);
    float distance_to_plane;
    if (! RayPlaneIntersect(ray, plane, distance_to_plane))
        return false;

    // Reduce the rest of the intersection computation to two dimensions by
    // projecting everything onto one of the principal coordinate planes.  Use
    // the component of the plane normal with the largest magnitude to find the
    // indices of the best plane to use.
    const int max_dim            = GetMaxAbsElementIndex(plane.normal);
    const Point2f p0_in_plane    = ToPoint2f(p0, max_dim);
    const Point2f p1_in_plane    = ToPoint2f(p1, max_dim);
    const Point2f p2_in_plane    = ToPoint2f(p2, max_dim);
    const Point3f inter          = ray.GetPoint(distance_to_plane);
    const Point2f inter_in_plane = ToPoint2f(inter, max_dim);

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

bool RaySphereIntersect(const Ray &ray, float radius, float &distance) {
    // Let:
    //   r = sphere radius
    //   P = starting point of ray
    //   D = ray direction
    //
    // A point on the ray:
    //   P + t * D   [t > 0]
    //
    // For any point S on the sphere:
    //   || S || == r
    //
    // Therefore:
    //   || (P + t * D) || == r
    //      square both sides
    //   (P + t * D) . (P + t * D) == r*r
    //      expand and refactor to get a quadratic equation:
    //
    //   a * t*t + b * t + c = 0
    //      where
    //         a = D . D
    //         b = 2 * (D . P)
    //         c = P . P - r*r

    // At^2 + Bt + C = 0
    const Vector3f p = ray.origin - Point3f::Zero();
    const float    a = ion::math::LengthSquared(ray.direction);
    const float    b = 2.f * ion::math::Dot(ray.direction, p);
    const float    c = ion::math::LengthSquared(p) - radius * radius;
    return SolveQuadratic_(a, b, c, distance);
}

bool RayCylinderIntersect(const Ray &ray, float radius, float &distance) {
    // First, rule out rays that are close to the Y axis.
    if (AreDirectionsClose(ion::math::Normalized(ray.direction),
                           Vector3f::AxisY(), Anglef::FromDegrees(.01f)))
        return false;

    // Now ignore the Y coordinate for the rest of this and do the math in 2D.
    //
    // Let:
    //   r = radius
    //   P = starting point of ray
    //   D = ray direction
    //
    // Any point on the ray:
    //   P + t * D   [t > 0]
    //
    // For any point C on the cylinder:
    //   || C || == r
    //
    // Therefore:
    //   || (P + t * D) || == r
    //      square both sides
    //   (P + t * D) . (P + t * D) == r*r
    //      expand and refactor to get a quadratic equation:
    //
    //   a * t*t + b * t + c = 0
    //      where
    //         a = D . D
    //         b = 2 * (D . P)
    //         c = P . P - r*r
    Point2f  p = ToPoint2f(ray.origin,     1);
    Vector2f d = ToVector2f(ray.direction, 1);

    const float a = ion::math::LengthSquared(d);
    const float b = 2.f * ion::math::Dot(d, Vector2f(p));
    const float c = ion::math::LengthSquared(Vector2f(p)) - radius * radius;
    return SolveQuadratic_(a, b, c, distance);
}

bool RayConeIntersect(const Ray &ray, const Point3f &apex,
                      const Vector3f &axis, const Anglef &half_angle,
                      float &distance) {
    // Let:
    //   h = half_angle
    //   A = apex
    //   V = axis vector
    //   P = starting point of ray
    //   D = ray direction
    //
    // Any point on the ray:
    //   P + t * D   [t > 0]
    //
    // For any point C on the cone:
    //   (C - A) . V = ||C - A|| cos(h)
    //
    // Therefore, the intersection point C satisfies:
    //   C + P + t * D   AND   (C - A) . V / ||C - A|| = cos(h)
    //     square the second equation:
    //   C + P + t * D   AND   [(C - A) .V]^2 / (C - A).(C - A) . V = cos^2(h)
    //     expand and refactor to get a quadratic equation:
    //   where
    //         a = (D.V)^2 - cos^2(h)
    //         b = 2 * [(D.V)(AP.V)-D.AP cos^2(h)]
    //         c = (AP.V)^2 - AP.AP cos^2(h)

    using ion::math::Dot;
    using ion::math::Length;
    using ion::math::Normalized;
    using ion::math::Square;

    // Normalize the direction vector to unit length to make the math work.
    const Vector3f norm_dir = Normalized(ray.direction);

    const float    cos2 = Square(ion::math::Cosine(half_angle));  // cos^2(h)
    const Vector3f pa   = ray.origin - apex;                      // AP
    const float    da   = Dot(norm_dir, axis);                    // D.V
    const float    pv   = Dot(pa, axis);                          // AP.V

    const float a = Square(da) - cos2;
    const float b = 2.f * (da * pv - Dot(norm_dir, pa) * cos2);
    const float c = Square(pv) - Dot(pa, pa) * cos2;

    // Check both roots if more than 1. One may be on the part of the cone that
    // does not exist (on the other side of the apex).
    float root0, root1;
    if (SolveQuadratic_(a, b, c, root0, root1)) {
        // Returns true if the root results in a point on the correct part of
        // the cone.
        const auto check_root = [&](float root){
            const Point3f cone_pt = ray.origin + root * norm_dir;
            return Dot(cone_pt - apex, axis) >= 0;
        };
        // Use the smaller distance that is on the correct side of the apex.
        if (check_root(root0))
            distance = root0;
        else if (check_root(root1))
            distance = root1;
        else
            return false;  // Neither hit on the correct side.

        // Scale the distance by the inverse of the ray direction length,
        distance /= Length(ray.direction);
        return true;
    }
    return false;
}

bool SphereBoundsIntersect(const Point3f &center, float radius,
                           const Bounds &bounds, float &distance) {
    // Arvo's algorithm from Graphics Gems: compute the square of the distance
    // from the sphere center to the box.
    float distance_squared = 0;
    const Point3f &min = bounds.GetMinPoint();
    const Point3f &max = bounds.GetMaxPoint();
    for (int dim = 0; dim < 3; ++dim) {
        if (center[dim] < min[dim]) {
            const float diff = min[dim] - center[dim];
            distance_squared += diff * diff;
        }
        else if (center[dim] > max[dim]) {
            const float diff = center[dim] - max[dim];
            distance_squared += diff * diff;
        }
    }
    if (distance_squared <= radius * radius) {
        distance = std::sqrt(distance_squared);
        return true;
    }
    return false;
}
