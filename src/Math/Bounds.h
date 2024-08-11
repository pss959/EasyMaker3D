//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"

/// A Bounds struct represents 3D bounds.
///
/// \ingroup Math
struct Bounds : public Range3f {
    /// Faces of bounds, ordered by dimension, then min/max.
    enum class Face { kLeft, kRight, kBottom, kTop, kBack, kFront };

    /// Default constructor creates empty bounds.
    Bounds() : Range3f() {}

    /// (Implicit) constructor to convert from a Range3f.
    Bounds(const Range3f &range) : Range3f(range) {}

    /// Constructor that creates Bounds with min and max points.
    Bounds(const Point3f &min, const Point3f &max) : Range3f(min, max) {}

    /// Constructor that creates Bounds centered on the origin with the given
    /// size.
    Bounds(const Vector3f &size) : Range3f(Point3f::Zero() - .5f * size,
                                           Point3f::Zero() + .5f * size) {}

    /// Returns the center point of the min or max bounds face in the given
    /// dimension.
    Point3f GetFaceCenter(Face face) const;

    /// Returns the dimension for a Face.
    static int GetFaceDim(Face face) { return static_cast<int>(face) / 2; }

    /// Returns true if a Face is on the maximum side of its dimension.
    static int IsFaceMax(Face face)  { return static_cast<int>(face) & 1; }

    /// Returns a Face for the given dimension/is_max pair.
    static Face GetFace(int dim, bool is_max);

    /// Returns the Face that contains the given point, which should be an
    /// intersection point on the bounds. If not, the front face is returned.
    Face GetFaceForPoint(const Point3f &point) const;

    /// Returns the unit normal to a Face.
    static Vector3f GetFaceNormal(Face face) {
        Vector3f normal(0, 0, 0);
        normal[GetFaceDim(face)] = IsFaceMax(face) ? 1.f : -1.f;
        return normal;
    }

    /// Stores all 8 corners of the Bounds in the given array.
    void GetCorners(Point3f corners[8]) const;

    /// Equality operator.
    bool operator==(const Bounds &b) const = default;

    /// Converts to a string to help with debugging. If use_min_max is true,
    /// the min/max values are printed rather than the center and size.
    Str ToString(bool use_min_max = false) const;
};
