#pragma once

#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include <Assert.h>

namespace SG {

//! \file
//! This file contains math-related items. It also defines convenience typedefs
//! for math-related Ion objects to make them more useful inside SG classes.

typedef ion::math::Anglef    Anglef;
typedef ion::math::Matrix2f  Matrix2f;
typedef ion::math::Matrix3f  Matrix3f;
typedef ion::math::Matrix4f  Matrix4f;
typedef ion::math::Point2f   Point2f;
typedef ion::math::Point2i   Point2i;
typedef ion::math::Point3f   Point3f;
typedef ion::math::Range2i   Range2i;
typedef ion::math::Range3f   Range3f;
typedef ion::math::Rotationf Rotationf;
typedef ion::math::Vector2f  Vector2f;
typedef ion::math::Vector2i  Vector2i;
typedef ion::math::Vector2ui Vector2ui;
typedef ion::math::Vector3f  Vector3f;
typedef ion::math::Vector3i  Vector3i;
typedef ion::math::Vector3ui Vector3ui;
typedef ion::math::Vector4f  Vector4f;
typedef ion::math::Vector4i  Vector4i;
typedef ion::math::Vector4ui Vector4ui;

//! A Ray struct represents a 3D ray.
struct Ray {
    Point3f  origin;     //!< Origin point of the ray.
    Vector3f direction;  //!< Ray direction, not necessarily normalized.

    //! The default constructor sets the origin to (0,0,0) and the direction to
    //! (0,0,-1);
    Ray() : origin(0, 0, 0), direction(0, 0, -1) {}

    //! Constructor setting both parts.
    Ray(const Point3f &p, const Vector3f &d) : origin(p), direction(d) {}

    //! Returns the point at parametric distance d along the ray.
    Point3f GetPoint(float d) const { return origin + d * direction; }
};

//! A Bounds struct represents 3D bounds.
struct Bounds : public Range3f {
    //! Faces of bounds, ordered by dimension, then min/max.
    enum class Face { kLeft, kRight, kBottom, kTop, kBack, kFront };

    //! Default constructor creates empty bounds.
    Bounds() : Range3f() {}

    //! Constructor that creates Bounds centered on the origin with the given
    //! size.
    Bounds(const Vector3f size) : Range3f(Point3f::Zero() - .5f * size,
                                          Point3f::Zero() + .5f * size) {}

    //! Returns the dimension for a Face.
    static int GetFaceDim(Face face) { return static_cast<int>(face) / 2; }

    //! Returns true if a Face is on the maximum side of its dimension.
    static int IsFaceMax(Face face)  { return static_cast<int>(face) & 1; }

    //! Returns a Face for the given dimension/is_max pair.
    static Face GetFace(int dim, bool is_max) {
        // Face enum values are ordered to make this work.
        ASSERT(dim >= 0 && dim <= 2);
        return static_cast<Face>(2 * dim + (is_max ? 1 : 0));
    }

    //! Returns the unit normal to a Face.
    static Vector3f GetFaceNormal(Face face) {
        Vector3f normal(0, 0, 0);
        normal[GetFaceDim(face)] = IsFaceMax(face) ? 1.f : -1.f;
        return normal;
    }
};

//! Transforms a Ray by a matrix. This does not normalize the resulting
//! direction vector, so that parametric distances are preserved.
Ray TransformRay(const Ray &ray, const Matrix4f &m);

//! Intersects a Ray with a Bounds. If they intersect, this sets distance to
//! the parametric distance to the closer intersection point and returns
//! true. Otherwise, it just returns false.
bool RayBoundsIntersect(const Ray &ray, const Bounds &bounds, float &distance);

//! Version of RayBoundsIntersect() that also returns the Face that was hit.
bool RayBoundsIntersectFace(const Ray &ray, const Bounds &bounds,
                            float &distance, Bounds::Face &face);

}  // namespace SG
