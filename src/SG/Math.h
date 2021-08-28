#pragma once

#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

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
};

//! A Bounds struct represents 3D bounds.
struct Bounds : public Range3f {
    // XXXX Anything to add here?
};

}  // namespace SG
