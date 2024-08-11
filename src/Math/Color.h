//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"

/// A color represents an RGB or RGBA color. The opacity is 1 by default, but
/// can be overridden.
///
/// \ingroup Math
struct Color : public Vector4f {
    /// Default constructor sets the color to opaque black.
    Color() : Vector4f(0, 0, 0, 1) {}

    /// Constructor from a Vector4f.
    Color(const Vector4f &v) : Vector4f(v) {}

    /// Constructor taking RGB values, setting alpha to 1.
    Color(float r, float g, float b) : Vector4f(r, g, b, 1) {}

    /// Constructor taking RGBA values.
    Color(float r, float g, float b, float a) : Vector4f(r, g, b, a) {}

    /// Returns opaque black.
    static Color Black() { return Color(); }

    /// Returns opaque white.
    static Color White() { return Color(1, 1, 1, 1); }

    /// Returns transparent black (all components 0).
    static Color Clear() { return Color(0, 0, 0, 0); }

    /// Creates a color from hue, saturation, and value.
    static Color FromHSV(float h, float s, float v);

    /// Returns a vector containing hue, saturation, and value for the Color.
    Vector3f ToHSV() const;

    /// Converts the color to a hex string in the format "#RRGGBBAA".
    Str ToHexString() const;

    /// Parses the given hex string in the format "#RRGGBBAA" or "#RRGGBB",
    /// storing the results in this instance. Returns false on error.
    bool FromHexString(const Str &str);

    /// Equality operator.
    bool operator==(const Color &c) const = default;

    Str ToString() const { return ToHexString(); }
};
