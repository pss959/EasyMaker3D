#pragma once

#include <ostream>

#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include "Util/String.h"

/// \file
/// This file defines math-related items, including convenience aliases for
/// math-related Ion objects to make them easier to use inside the application.
///
/// \ingroup Math

using Anglef    = ion::math::Anglef;
using Matrix2f  = ion::math::Matrix2f;
using Matrix3f  = ion::math::Matrix3f;
using Matrix4f  = ion::math::Matrix4f;
using Point2f   = ion::math::Point2f;
using Point2i   = ion::math::Point2i;
using Point2ui  = ion::math::Point2ui;
using Point3f   = ion::math::Point3f;
using Range1f   = ion::math::Range1f;
using Range2f   = ion::math::Range2f;
using Range2ui  = ion::math::Range2ui;
using Range3f   = ion::math::Range3f;
using Rotationf = ion::math::Rotationf;
using Vector2f  = ion::math::Vector2f;
using Vector2i  = ion::math::Vector2i;
using Vector2ui = ion::math::Vector2ui;
using Vector3f  = ion::math::Vector3f;
using Vector3i  = ion::math::Vector3i;
using Vector3ui = ion::math::Vector3ui;
using Vector4f  = ion::math::Vector4f;
using Vector4i  = ion::math::Vector4i;
using Vector4ui = ion::math::Vector4ui;

template <int DIM, typename T> using VectorBase = ion::math::VectorBase<DIM, T>;

using GIndex   = unsigned int;  ///< Type used for any geometry index.
using Viewport = Range2ui;      ///< Used for viewing operations.

// ----------------------------------------------------------------------------
// Output helpers.
// ----------------------------------------------------------------------------

/// Overrides the standard Ion string printing version to be more compact and
/// to round to a reasonable precision. This is specialized for some other
/// types in the source file.
template <typename T> Str ToStr(const T &t) {
    return Util::ToString(t);
}

/// A Stringable is any class that supports ToString().
template <typename T>
concept Stringable = requires(T v)
{
    {v.ToString()} -> std::convertible_to<Str>;
};

/// Output operator for a Stringable.
template <Stringable T>
inline std::ostream& operator<<(std::ostream& out, const T &t) {
    return out << t.ToString();
}

// ----------------------------------------------------------------------------
// Conversion helpers.
// ----------------------------------------------------------------------------

/// Converts memory pointed to by \p ptr to the templated type, incrementing
/// the pointer to the next byte.
template <typename T> T FromBinary(const uint8 *&ptr) {
    const T val = *reinterpret_cast<const T *>(ptr);
    ptr += sizeof(val);
    return val;
}
