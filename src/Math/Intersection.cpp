#include "Math/Intersection.h"

#include <algorithm>
#include <limits>

#include "Math/Linear.h"

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
