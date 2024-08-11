//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"

/// 3D plane.
///
/// \ingroup Math
struct Plane {
    Vector3f normal;    ///< Plane Normal, pointing to positive half-space.
    float    distance;  ///< Signed distance from origin.

    /// The default constructor creates the XY plane.
    Plane();

    /// Constructs from signed distance and normal.
    Plane(float dist, const Vector3f &norm);

    /// Constructs from point and normal.
    Plane(const Point3f &point, const Vector3f &norm);

    /// Constructs from three points.
    Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2);

    /// Returns true if this is the default plane.
    bool IsDefault() const {
        return distance == 0 && normal == Vector3f::AxisZ();
    }

    /// Projects a point onto the plane, returning the result.
    Point3f ProjectPoint(const Point3f &p) const;

    /// Projects a vector onto the plane, returning the result.
    Vector3f ProjectVector(const Vector3f &v) const;

    /// Projects a Rotationf onto the plane, returning the result.
    Rotationf ProjectRotation(const Rotationf &rot) const;

    /// Returns the signed distance of the given point from the plane.
    float GetDistanceToPoint(const Point3f &p) const;

    /// Mirrors a point about the plane, returning the result.
    Point3f MirrorPoint(const Point3f &p) const;

    /// Gets the 4 coefficients of the plane equation (ax + by + cz + d = 0).
    Vector4f GetCoefficients() const;

    /// Equality operator.
    bool operator==(const Plane &p) const = default;

    /// Converts to a string to help with debugging.
    Str ToString() const;
};
