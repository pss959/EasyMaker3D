#pragma once

#include <string>

#include "Math/Types.h"

/// \file
/// This file contains functions for converting Math types to character strings.
///
/// \ingroup Math

namespace Math {

/// Converts a float with output precision.
///
/// \ingroup Math
Str ToString(float f, float precision);

/// Converts a vector with output precision.
///
/// \ingroup Math
Str ToString(const VectorBase<2, float> &v, float precision);

#if ! DOXYGEN  // Doxygen cannot handle the template overload with alias?
/// Converts a vector with output precision.
///
/// \ingroup math
Str ToString(const VectorBase<3, float> &v, float precision);

/// Converts a rotation with reasonable output precision.
///
/// \ingroup math
Str ToString(const Rotationf &r);

/// Converts a matrix with output precision, using multiple lines for clarity.
///
/// \ingroup math
Str ToString(const Matrix4f &m, float precision);
#endif

}  // namespace Math
