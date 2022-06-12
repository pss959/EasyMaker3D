#include "Math/Linear.h"

#include <ion/math/transformutils.h>

#include <algorithm>

// ----------------------------------------------------------------------------
// Transformation functions.
// ----------------------------------------------------------------------------

Matrix4f GetTransformMatrix(const Vector3f &scale, const Rotationf &rot,
                            const Vector3f &trans) {
    return ion::math::TranslationMatrix(trans) *
        ion::math::RotationMatrixH(rot) * ion::math::ScaleMatrixH(scale);
}

Ray TransformRay(const Ray &ray, const Matrix4f &m) {
    return Ray(m * ray.origin, m * ray.direction);
}

Plane TransformPlane(const Plane &plane, const Matrix4f &m) {
    // Transform a point on the plane.
    return Plane(m * Point3f(plane.distance * plane.normal), m * plane.normal);
}

Bounds ScaleBounds(const Bounds &bounds, const Vector3f &scale) {
    const Point3f sp(scale);
    return Bounds(sp * bounds.GetMinPoint(), sp * bounds.GetMaxPoint());
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

Point3f ToPrintCoords(const Point3f &p) {
    // Be careful not to negate 0.
    return Point3f(p[0], p[2] ? -p[2] : 0, p[1]);
}

Vector3f ToPrintCoords(const Vector3f &v) {
    // Be careful not to negate 0.
    return Vector3f(v[0], v[2] ? -v[2] : 0, v[1]);
}

// ----------------------------------------------------------------------------
// Viewing functions.
// ----------------------------------------------------------------------------

float GetAspectRatio(const Viewport &viewport) {
    const auto &size = viewport.GetSize();
    return static_cast<float>(size[0]) / size[1];
}

Matrix4f GetProjectionMatrix(const Frustum &frustum) {
    const float tan_l = tanf(frustum.fov_left.Radians());
    const float tan_r = tanf(frustum.fov_right.Radians());
    const float tan_u = tanf(frustum.fov_up.Radians());
    const float tan_d = tanf(frustum.fov_down.Radians());

    const float nr = frustum.pnear;
    const float fr = frustum.pfar;

    const float inv_tan_lr = 1 / (tan_r - tan_l);
    const float inv_tan_du = 1 / (tan_u - tan_d);
    const float inv_nf     = 1 / (fr - nr);

    return Matrix4f(
        2 * inv_tan_lr, 0,              (tan_r + tan_l) * inv_tan_lr, 0,
        0,              2 * inv_tan_du, (tan_u + tan_d) * inv_tan_du, 0,
        0,              0,              -fr * inv_nf,    -(fr * nr) * inv_nf,
        0,              0,              -1,              0);
}

Matrix4f GetViewMatrix(const Frustum &frustum) {
    return ion::math::RotationMatrixH(-frustum.orientation) *
        ion::math::TranslationMatrix(-frustum.position);
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

bool AreClose(const Point3f &a, const Point3f &b, float tolerance) {
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

Anglef AbsAngle(const Anglef &angle) {
    return angle.Radians() < 0 ? -angle : angle;
}

Anglef NormalizedAngle(const Anglef &angle) {
    return ion::math::WrapTwoPi(angle);
}

Anglef RotationAngle(const Rotationf &rot) {
    Vector3f axis;
    Anglef   angle;
    rot.GetAxisAndAngle(&axis, &angle);
    return angle;
}

Rotationf RotationFromMatrix(const Matrix4f &mat) {
    return Rotationf::FromRotationMatrix(ion::math::GetRotationMatrix(mat));
}

Rotationf RotationDifference(const Rotationf &r0, const Rotationf &r1) {
    return -r0 * r1;
}

float SignedDistance(const Point3f &p, const Vector3f &v) {
    return ion::math::Dot(v, Vector3f(p));
}

Vector3f ComputeNormal(const Point3f &p0, const Point3f &p1,
                       const Point3f &p2) {
    return ion::math::Normalized(ion::math::Cross(p1 - p0, p2 - p0));
}

Vector3f ComputeNormal(const std::vector<Point3f> &points) {
    ASSERT(points.size() >= 3U);
    if (points.size() == 3U) {
        return ComputeNormal(points[0], points[1], points[2]);
    }
    else {
        // General case. Use Newell's method.
        Vector3f normal(0, 0, 0);
        for (size_t i0 = 0; i0 < points.size(); ++i0) {
            const size_t i1 = (i0 + 1) % points.size();
            const Vector3f diff = points[i0] - points[i1];
            const Vector3f sum  = Vector3f(points[i0] + points[i1]);
            normal += Vector3f(diff[1] * sum[2],
                               diff[2] * sum[0],
                               diff[0] * sum[1]);
        }
        return ion::math::Normalized(normal);
    }
}

float ComputeArea(const std::vector<Point3f> &points) {
    Vector3f sum(0, 0, 0);
    for (size_t i = 0; i < points.size(); ++i) {
        const size_t j = i == 0 ? points.size() - 1 : i - 1;
        sum += ion::math::Cross(Vector3f(points[j]), Vector3f(points[i]));
    }
    return .5f * ion::math::Dot(ComputeNormal(points), sum);
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
    if (AreClose(denom, 0.f, 1e-7f))  // Should never happen.
        return false;

    const float alpha = (d11 * d20 - d01 * d21) / denom;
    const float beta  = (d00 * d21 - d01 * d20) / denom;
    if (alpha < 0.f || beta < 0.f || alpha + beta > 1.f)
        return false;

    bary.Set(1.f - alpha - beta, alpha, beta);
    return true;
}

Point3f GetClosestPointOnLine(const Point3f &p, const Point3f &line_pt,
                              const Vector3f &line_dir) {
    const float length_squared = ion::math::LengthSquared(line_dir);
    ASSERT(length_squared > 0);
    const float projection = ion::math::Dot((p - line_pt), line_dir);
    return line_pt + (projection / length_squared) * line_dir;
}

bool GetClosestLinePoints(const Point3f &p0, const Vector3f &dir0,
                          const Point3f &p1, const Vector3f &dir1,
                          Point3f &closest_pt0, Point3f &closest_pt1) {
    using ion::math::Dot;

    closest_pt0.Set(0, 0, 0);
    closest_pt1.Set(0, 0, 0);

    const float a = Dot(dir0, dir0);
    const float b = Dot(dir0, dir1);
    const float e = Dot(dir1, dir1);
    const float d = a * e - b * b;

    // Check for parallel lines.
    if (d == 0.0f)
        return false;

    // Not parallel.
    const Vector3f r = p0 - p1;
    const float c = Dot(dir0, r);
    const float f = Dot(dir1, r);

    // Compute the parametric distance on each line from the points.
    const float dist0 = (b * f - c * e) / d;
    const float dist1 = (a * f - c * b) / d;

    closest_pt0 = p0 + dist0 * dir0;
    closest_pt1 = p1 + dist1 * dir1;

    return true;
}

// ----------------------------------------------------------------------------
// Min/Max.
// ----------------------------------------------------------------------------

Vector2f MinComponents(const Vector2f &v0, const Vector2f &v1) {
    return Vector2f(std::min(v0[0], v1[0]), std::min(v0[1], v1[1]));
}

Vector2f MaxComponents(const Vector2f &v0, const Vector2f &v1) {
    return Vector2f(std::max(v0[0], v1[0]), std::max(v0[1], v1[1]));
}

Vector3f MinComponents(const Vector3f &v0, const Vector3f &v1) {
    return Vector3f(std::min(v0[0], v1[0]), std::min(v0[1], v1[1]),
                    std::min(v0[2], v1[2]));
}

Vector3f MaxComponents(const Vector3f &v0, const Vector3f &v1) {
    return Vector3f(std::max(v0[0], v1[0]), std::max(v0[1], v1[1]),
                    std::max(v0[2], v1[2]));
}

Point2f MinComponents(const Point2f &v0, const Point2f &v1) {
    return Point2f(std::min(v0[0], v1[0]), std::min(v0[1], v1[1]));
}

Point2f MaxComponents(const Point2f &v0, const Point2f &v1) {
    return Point2f(std::max(v0[0], v1[0]), std::max(v0[1], v1[1]));
}

Point3f MinComponents(const Point3f &v0, const Point3f &v1) {
    return Point3f(std::min(v0[0], v1[0]), std::min(v0[1], v1[1]),
                   std::min(v0[2], v1[2]));
}

Point3f MaxComponents(const Point3f &v0, const Point3f &v1) {
    return Point3f(std::max(v0[0], v1[0]), std::max(v0[1], v1[1]),
                   std::max(v0[2], v1[2]));
}

// ----------------------------------------------------------------------------
// Clamping.
// ----------------------------------------------------------------------------

template <int DIM, typename T>
static T Clamp_(const T &v, const T &min, const T &max) {
    T clamped;
    for (int i = 0; i < DIM; ++i)
        clamped[i] = std::clamp(v[i], min[i], max[i]);
    return clamped;
}

float Clamp(float v, float min, float max) {
    return std::clamp(v, min, max);
}
Vector2f Clamp(const Vector2f &v, const Vector2f &min, const Vector2f &max) {
    return Clamp_<2>(v, min, max);
}
Vector3f Clamp(const Vector3f &v, const Vector3f &min, const Vector3f &max) {
    return Clamp_<3>(v, min, max);
}
Point2f  Clamp(const Point2f &v, const Point2f &min, const Point2f &max) {
    return Clamp_<2>(v, min, max);
}
Point3f  Clamp(const Point3f &v, const Point3f &min, const Point3f &max) {
    return Clamp_<3>(v, min, max);
}
