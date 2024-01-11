#pragma once

#include "Math/Types.h"

/// A Ray struct represents a 3D ray.
///
/// \ingroup Math
struct Ray {
    Point3f  origin;     ///< Origin point of the ray.
    Vector3f direction;  ///< Ray direction, not necessarily normalized.

    /// The default constructor sets the origin to (0,0,0) and the direction to
    /// (0,0,-1);
    Ray() : origin(0, 0, 0), direction(0, 0, -1) {}

    /// Constructor setting both parts.
    Ray(const Point3f &p, const Vector3f &d) : origin(p), direction(d) {}

    /// Returns the point at parametric distance d along the ray.
    Point3f GetPoint(float d) const { return origin + d * direction; }

    /// Equality operator.
    bool operator==(const Ray &r) const = default;

    /// Converts to a string to help with debugging.
    Str ToString() const;
};
