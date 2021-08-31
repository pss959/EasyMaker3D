#include "Math/Linear.h"

#include <ion/math/transformutils.h>

#include <algorithm>

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
