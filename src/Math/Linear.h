#pragma once

#include "Assert.h"
#include "Math/Types.h"

//! \file
//! This file defines some basic linear algebra functions.
//! \ingroup Math

// ----------------------------------------------------------------------------
// Transformation functions.
// ----------------------------------------------------------------------------

//! Transforms a Ray by a matrix. This does not normalize the resulting
//! direction vector, so that parametric distances are preserved.
Ray TransformRay(const Ray &ray, const Matrix4f &m);

//! Transforms a Bounds by a matrix, returning a new (aligned) Bounds that
//! encloses the result.
Bounds TransformBounds(const Bounds &bounds, const Matrix4f &m);

// ----------------------------------------------------------------------------
// General linear algebra functions.
// ----------------------------------------------------------------------------

//! Returns true if two values are close enough to each other within a
// tolerance.
bool AreClose(float a, float b, float tolerance = .0001f);

//! Returns true if two vectors are close enough to each other within a
// tolerance, which is the square of the distance.
bool AreClose(const Vector3f &a, const Vector3f &b, float tolerance = .0001f);

//! Returns the index of the minimum element of a Vector3f.
int GetMinElementIndex(const Vector3f &v);

//! Returns the index of the maximum element of a Vector3f.
int GetMaxElementIndex(const Vector3f &v);

//! Returns the index of the minimum element (by absolute value) of a Vector3f.
int GetMinAbsElementIndex(const Vector3f &v);

//! Returns the index of the maximum element (by absolute value) of a Vector3f.
int GetMaxAbsElementIndex(const Vector3f &v);

//! Returns the coordinate axis for the given dimension (0, 1, or 2).
inline Vector3f GetAxis(int dim) {
    ASSERT(dim >= 0 && dim <= 2);
    Vector3f axis = Vector3f::Zero();
    axis[dim] = 1.f;
    return axis;
}

//! Computes the normal to a triangle defined by three points.
Vector3f ComputeNormal(const Point3f &p0, const Point3f &p1, const Point3f &p2);

//! Computes and returns barycentric coordinates for point p with respect to
//! triangle ABC. Returns false if the point is outside the triangle.
bool ComputeBarycentric(const Point2f &p, const Point2f & a,
                        const Point2f &b, const Point2f &c, Vector3f &bary);

//! \name Clamping
//! Each of these clamps a value of some type to a range. Vectors and
//! points are clampled component-wise.
//!@{
float    Clamp(float v, float min, float max);
Vector2f Clamp(const Vector2f &v, const Vector2f &min, const Vector2f &max);
Vector3f Clamp(const Vector3f &v, const Vector3f &min, const Vector3f &max);
Point2f  Clamp(const Point2f &v, const Point2f &min, const Point2f &max);
Point3f  Clamp(const Point3f &v, const Point3f &min, const Point3f &max);
//!@}

//! Linearly interpolates a value of some type based on the t interpolant in
//! the range (0,1).
template <typename T> T Lerp(float t, const T &min, const T &max) {
    return min + t * (max - min);
}
