#pragma once

#include <cmath>

#include "Math/Types.h"
#include "Util/Assert.h"

/// \file
/// This file defines some basic linear algebra functions.
/// \ingroup Math

// ----------------------------------------------------------------------------
// Transformation functions.
// ----------------------------------------------------------------------------

/// Computes and returns a matrix that scales, rotates, and translates.
Matrix4f GetTransformMatrix(const Vector3f &scale, const Rotationf &rot,
                            const Vector3f &trans);

/// Transforms a Ray by a matrix. This does not normalize the resulting
/// direction vector, so that parametric distances are preserved.
Ray TransformRay(const Ray &ray, const Matrix4f &m);

/// Scales a Bounds by the given factors, returning a new Bounds. Note that if
/// the Bounds is not centered on the origin, this will move it.
Bounds ScaleBounds(const Bounds &bounds, const Vector3f &scale);

/// Transforms a Bounds by a matrix, returning a new (aligned) Bounds that
/// encloses the result.
Bounds TransformBounds(const Bounds &bounds, const Matrix4f &m);

/// Composes two rotations in the proper order so r1 has more local effect than
/// r0.
inline Rotationf ComposeRotations(const Rotationf &r0, const Rotationf &r1) {
    return r1 * r0;
}

// ----------------------------------------------------------------------------
// Viewing functions.
// ----------------------------------------------------------------------------

/// Returns the aspect ratio for a viewport.
float GetAspectRatio(const Viewport &viewport);

/// Returns the projection matrix corresponding to a Frustum.
Matrix4f GetProjectionMatrix(const Frustum &frustum);

/// Returns the view matrix corresponding to a Frustum.
Matrix4f GetViewMatrix(const Frustum &frustum);

// ----------------------------------------------------------------------------
// General linear algebra functions.
// ----------------------------------------------------------------------------

/// This templated function allows initialization of a float or vector variable
/// to zero. It returns a zero-initialized item.
template <typename T> inline T        ZeroInit();
template <>           inline float    ZeroInit() { return 0.f; }
template <>           inline Vector2f ZeroInit() { return Vector2f::Zero(); }

/// This templated function allows initialization of a float or vector variable
/// to one. It returns a one-initialized item.
template <typename T> inline T        OneInit();
template <>           inline float    OneInit() { return 1.f; }
template <>           inline Vector2f OneInit() { return Vector2f(1, 1); }

/// Rounds a floating point value to the given precision. For example, calling
/// \code{.cpp}
/// RoundToPrecision(1.2345f, .01f)
/// \endcode
/// will return 1.23f.
inline float RoundToPrecision(float value, float precision) {
    return precision * std::roundf(value / precision);
}

/// Returns true if two values are close enough to each other within a
// tolerance.
bool AreClose(float a, float b, float tolerance = .0001f);

/// Returns true if two vectors are close enough to each other within a
// tolerance, which is the square of the distance.
bool AreClose(const Vector3f &a, const Vector3f &b, float tolerance = .0001f);

/// Returns true if two points are close enough to each other within a
// tolerance, which is the square of the distance.
bool AreClose(const Point3f &a, const Point3f &b, float tolerance = .0001f);

/// Returns the index of the minimum element of a Vector3f.
int GetMinElementIndex(const Vector3f &v);

/// Returns the index of the maximum element of a Vector3f.
int GetMaxElementIndex(const Vector3f &v);

/// Returns the index of the minimum element (by absolute value) of a Vector3f.
int GetMinAbsElementIndex(const Vector3f &v);

/// Returns the index of the maximum element (by absolute value) of a Vector3f.
int GetMaxAbsElementIndex(const Vector3f &v);

/// Returns the coordinate axis for the given dimension (0, 1, or 2). The scale
/// value (default 1) is used for the axis length.
inline Vector3f GetAxis(int dim, float scale = 1.f) {
    ASSERT(dim >= 0 && dim <= 2);
    Vector3f axis = Vector3f::Zero();
    axis[dim] = scale;
    return axis;
}

/// Returns the absolute value of an angle.
Anglef AbsAngle(const Anglef &angle);

/// Returns the angle part of a rotation.
Anglef RotationAngle(const Rotationf &rot);

/// Returns the difference between two rotations. That is, this returns rdiff
/// such that r0 * diff = r1.
Rotationf RotationDifference(const Rotationf &r0, const Rotationf &r1);

/// Computes the normal to a triangle defined by three points.
Vector3f ComputeNormal(const Point3f &p0, const Point3f &p1, const Point3f &p2);

/// Computes the normal to a polygon defined by a vector of points. Should
/// work for concave as well as convex polygons.
Vector3f ComputeNormal(const std::vector<Point3f> &points);

/// Returns the area of the planar 3D polygon formed by the given points.
float ComputeArea(const std::vector<Point3f> &points);

/// Computes and returns barycentric coordinates for point p with respect to
/// triangle ABC. Returns false if the point is outside the triangle.
bool ComputeBarycentric(const Point2f &p, const Point2f & a,
                        const Point2f &b, const Point2f &c, Vector3f &bary);

/// Finds the closest point to the given point on an infinite line specified by
/// a point and vector.
Point3f GetClosestPointOnLine(const Point3f &p, const Point3f &line_pt,
                              const Vector3f &line_dir);

/// Given two 3D lines each defined by point and direction vector, this sets
/// closest_pt0 and closest_pt1 to the points on the lines that are closest
/// together. If the lines are parallel, this just returns false.
bool GetClosestLinePoints(const Point3f &p0, const Vector3f &dir0,
                          const Point3f &p1, const Vector3f &dir1,
                          Point3f &closest_pt0, Point3f &closest_pt1);

/// Builds a Range2f from a center and size.
inline Range2f BuildRange(const Point2f &center, const Vector2f &size) {
    return Range2f(center - .5f * size, center + .5f * size);
}

/// \name Min/Max
/// Each of these returns the component-wise minimum or maximum of two values.
///@{
Vector2f MinComponents(const Vector2f &v0, const Vector2f &v1);
Vector2f MaxComponents(const Vector2f &v0, const Vector2f &v1);
Vector3f MinComponents(const Vector3f &v0, const Vector3f &v1);
Vector3f MaxComponents(const Vector3f &v0, const Vector3f &v1);
Point2f  MinComponents(const Point2f  &v0, const Point2f  &v1);
Point2f  MaxComponents(const Point2f  &v0, const Point2f  &v1);
Point3f  MinComponents(const Point3f  &v0, const Point3f  &v1);
Point3f  MaxComponents(const Point3f  &v0, const Point3f  &v1);
///@}

/// \name Clamping
/// Each of these clamps a value of some type to a range. Vectors and
/// points are clampled component-wise.
///@{
float    Clamp(float v, float min, float max);
Vector2f Clamp(const Vector2f &v, const Vector2f &min, const Vector2f &max);
Vector3f Clamp(const Vector3f &v, const Vector3f &min, const Vector3f &max);
Point2f  Clamp(const Point2f &v, const Point2f &min, const Point2f &max);
Point3f  Clamp(const Point3f &v, const Point3f &min, const Point3f &max);
///@}

/// Linearly interpolates a value of some type based on the t interpolant in
/// the range (0,1).
template <typename T> T Lerp(float t, const T &min, const T &max) {
    return min + t * (max - min);
}
