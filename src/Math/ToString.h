#pragma once

#include <string>

#include <ion/math/matrix.h>
#include <ion/math/vector.h>

/// \file
/// This file contains functions for converting Math types to character strings.
///
/// \ingroup Math

namespace Math {

/// Converts a float with output precision.
///
/// \ingroup Math
std::string ToString(float f, float precision);

/// Converts a vector with output precision.
///
/// \ingroup Math
std::string ToString(const ion::math::VectorBase<2, float> &v, float precision);

/// Converts a vector with output precision.
///
/// \ingroup math
std::string ToString(const ion::math::VectorBase<3, float> &v, float precision);

/// Converts a matrix with output precision, using multiple lines for clarity.
///
/// \ingroup math
std::string ToString(const ion::math::Matrix4f &m, float precision);

}  // namespace Math
