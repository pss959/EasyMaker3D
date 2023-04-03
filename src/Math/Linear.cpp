#include "Math/Linear.h"

#include <ion/math/angleutils.h>
#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include <algorithm>

// ----------------------------------------------------------------------------
// Dimension conversion functions.
// ----------------------------------------------------------------------------

Vector2f ToVector2f(const Vector3f &v, int dim) {
    return ion::math::WithoutDimension(v, dim);
}

Point2f ToPoint2f(const Point3f &p, int dim) {
    return ion::math::WithoutDimension(p, dim);
}

Range2f ToRange2f(const Range3f &r, int dim) {
    return Range2f(ToPoint2f(r.GetMinPoint(), dim),
                   ToPoint2f(r.GetMaxPoint(), dim));
}

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
    using ion::math::Inverse;
    using ion::math::Normalized;
    using ion::math::Transpose;

    // Transform a point on the plane and the normal.
    const Point3f p = m * Point3f(plane.distance * plane.normal);
    Vector3f      n = Normalized(Transpose(Inverse(m)) * plane.normal);

    // Compute the distance of the point and Adjust it if it is close to the
    // origin.
    float d = SignedDistance(p, n);
    if (AreClose(d, 0))
        d = 0;

    // Adjust a unit normal close to a principal axis.
    if      (AreClose(n[0], -1.f))
        n = -Vector3f::AxisX();
    else if (AreClose(n[0],  1.f))
        n =  Vector3f::AxisX();
    else if (AreClose(n[1], -1.f))
        n = -Vector3f::AxisY();
    else if (AreClose(n[1],  1.f))
        n =  Vector3f::AxisY();
    else if (AreClose(n[2], -1.f))
        n = -Vector3f::AxisZ();
    else if (AreClose(n[2],  1.f))
        n =  Vector3f::AxisZ();
    return Plane(d, n);
}

Bounds ScaleBounds(const Bounds &bounds, const Vector3f &scale) {
    const Point3f sp(scale);
    return Bounds(sp * bounds.GetMinPoint(), sp * bounds.GetMaxPoint());
}

Bounds TranslateBounds(const Bounds &bounds, const Vector3f &trans) {
    return Bounds(bounds.GetMinPoint() + trans, bounds.GetMaxPoint() + trans);
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

void ScalePointsToSize(const Vector2f &target, std::vector<Point2f> &points) {
    Range2f bounds;
    for (const auto &p: points)
        bounds.ExtendByPoint(p);
    const Vector2f size = bounds.GetSize();
    if (! AreClose(size[0], target[0]) || ! AreClose(size[1], target[1])) {
        const float sx = target[0] / size[0];
        const float sy = target[1] / size[1];
        for (auto &p: points) {
            p[0] *= sx;
            p[1] *= sy;
        }
    }
}

void ScalePointsToSize(const Vector3f &target, std::vector<Point3f> &points) {
    Range3f bounds;
    for (const auto &p: points)
        bounds.ExtendByPoint(p);
    const Vector3f size = bounds.GetSize();
    if (! AreClose(size[0], target[0]) ||
        ! AreClose(size[1], target[1]) ||
        ! AreClose(size[2], target[2])) {
        const Vector3f scale = target / size;
        for (auto &p: points)
            p = ScalePoint(p, scale);
    }
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
    return std::abs(b - a) <= tolerance;
}

bool AreClose(double a, double b, double tolerance) {
    return std::abs(b - a) <= tolerance;
}

bool AreClose(const Vector3f &a, const Vector3f &b, float tolerance) {
    return ion::math::LengthSquared(b - a) <= tolerance;
}

bool AreClose(const Point3f &a, const Point3f &b, float tolerance) {
    return ion::math::LengthSquared(b - a) <= tolerance;
}

bool AreClose(const Anglef &a, const Anglef &b, const Anglef &tolerance) {
    return AreClose(a.Radians(), b.Radians(), tolerance.Radians());
}

bool AreDirectionsClose(const Vector3f dir0, const Vector3f dir1,
                        const Anglef &tolerance_angle) {
    return ion::math::AngleBetween(dir0, dir1) <= tolerance_angle;
}

template <int DIM>
int GetMinElementIndex(const VectorBase<DIM, float> &v) {
    int   min_index = 0;
    float min_value = v[0];
    for (int i = 1; i < DIM; ++i) {
        if (v[i] < min_value) {
            min_index = i;
            min_value = v[i];
        }
    }
    return min_index;
}

template <int DIM>
int GetMaxElementIndex(const VectorBase<DIM, float> &v) {
    int   max_index = 0;
    float max_value = v[0];
    for (int i = 1; i < DIM; ++i) {
        if (v[i] > max_value) {
            max_index = i;
            max_value = v[i];
        }
    }
    return max_index;
}

template <int DIM>
int GetMinAbsElementIndex(const VectorBase<DIM, float> &v) {
    ion::math::Vector<DIM, float> av;
    for (int i = 0; i < DIM; ++i)
	av[i] = std::abs(v[i]);
    return GetMinElementIndex(av);
}

template <int DIM>
int GetMaxAbsElementIndex(const VectorBase<DIM, float> &v) {
    ion::math::Vector<DIM, float> av;
    for (int i = 0; i < DIM; ++i)
	av[i] = std::abs(v[i]);
    return GetMaxElementIndex(av);
}

// Instantiate for all needed vector dimensions.
template int GetMinElementIndex(const VectorBase<2, float> &v);
template int GetMinElementIndex(const VectorBase<3, float> &v);
template int GetMinElementIndex(const VectorBase<4, float> &v);
template int GetMaxElementIndex(const VectorBase<2, float> &v);
template int GetMaxElementIndex(const VectorBase<3, float> &v);
template int GetMaxElementIndex(const VectorBase<4, float> &v);
template int GetMinAbsElementIndex(const VectorBase<2, float> &v);
template int GetMinAbsElementIndex(const VectorBase<3, float> &v);
template int GetMinAbsElementIndex(const VectorBase<4, float> &v);
template int GetMaxAbsElementIndex(const VectorBase<2, float> &v);
template int GetMaxAbsElementIndex(const VectorBase<3, float> &v);
template int GetMaxAbsElementIndex(const VectorBase<4, float> &v);

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

Vector3f RotationAxis(const Rotationf &rot) {
    Vector3f axis;
    Anglef   angle;
    rot.GetAxisAndAngle(&axis, &angle);
    return axis;
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

float ComputeArea(const Point3f &p0, const Point3f &p1, const Point3f &p2) {
    return ComputeArea(std::vector<Point3f>{p0, p1, p2});
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

    // Use doubles here for extra precision.
    const double d00 = Dot(v0, v0);
    const double d01 = Dot(v0, v1);
    const double d11 = Dot(v1, v1);
    const double d20 = Dot(v2, v0);
    const double d21 = Dot(v2, v1);

    const double denom = d00 * d11 - d01 * d01;
    if (AreClose(denom, 0, 1e-12))  // Should never happen.
        return false;

    const double alpha = (d11 * d20 - d01 * d21) / denom;
    const double beta  = (d00 * d21 - d01 * d20) / denom;
    if (alpha < 0.f || beta < 0.f || alpha + beta > 1.f)
        return false;

    bary.Set(1 - alpha - beta, alpha, beta);
    return true;
}

Point3f GetClosestPointOnLine(const Point3f &p, const Point3f &line_pt,
                              const Vector3f &line_dir) {
    return line_pt + ion::math::Projection(p - line_pt, line_dir);
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

bool IsNearLineSegment(const Point2f &p, const Point2f &end0,
                       const Point2f &end1, float tolerance) {
    // Do this in 3D since the function already exists.
    Point3f close0, close1;
    if (! GetClosestLinePoints(Point3f(p, 0), Vector3f(0, 0, 1),
                               Point3f(end0, 0),
                               ion::math::Normalized(Vector3f(end1 - end0, 0)),
                               close0, close1))
        return false;  // Should never happen - can't be parallel.

    auto to2 = [](const Point3f &p){ return Point2f(p[0], p[1]); };
    const Point2f c0 = to2(close0);
    const Point2f c1 = to2(close1);
    if (ion::math::Distance(c0, c1) <= tolerance) {
        // Make sure the point is between end0 and end1.
        if (ion::math::Dot(c1 - end0, end1 - end0) > 0 &&
            ion::math::Dot(c1 - end1, end0 - end1) > 0)
            return true;
    }

    return false;
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

int Clamp(int v, int min, int max) {
    return std::clamp(v, min, max);
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

// ----------------------------------------------------------------------------
// Interpolation.
// ----------------------------------------------------------------------------

int LerpInt(float t, int min, int max) {
    return min + static_cast<int>(t * (max - min));
}
