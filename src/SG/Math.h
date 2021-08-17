#pragma once

#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

namespace SG {

//! \file This file contains convenience typedefs for math-related Ion objects
//! to make them more useful inside SG classes.
//!
//! \ingroup SG

typedef ion::math::Vector2f  Vector2f;
typedef ion::math::Vector3f  Vector3f;
typedef ion::math::Vector4f  Vector4f;
typedef ion::math::Vector2i  Vector2i;
typedef ion::math::Vector3i  Vector3i;
typedef ion::math::Vector4i  Vector4i;
typedef ion::math::Vector2ui Vector2ui;
typedef ion::math::Vector3ui Vector3ui;
typedef ion::math::Vector4ui Vector4ui;
typedef ion::math::Anglef    Anglef;
typedef ion::math::Matrix2f  Matrix2f;
typedef ion::math::Matrix3f  Matrix3f;
typedef ion::math::Matrix4f  Matrix4f;
typedef ion::math::Rotationf Rotationf;

}  // namespace SG
