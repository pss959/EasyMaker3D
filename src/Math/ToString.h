#pragma once

#include <string>

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

}  // namespace Math
