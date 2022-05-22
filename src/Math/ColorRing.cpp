#include "Math/ColorRing.h"

#include <ion/math/angleutils.h>

#include "Math/Linear.h"

const float ColorRing::kOuterRadius   = 1;
const float ColorRing::kInnerRadius   = .45f;
const float ColorRing::kMinSaturation = .25f;
const float ColorRing::kMaxSaturation = .50f;
const float ColorRing::kMinValue      = .90f;
const float ColorRing::kMaxValue      = .95f;

Color ColorRing::GetColorForPoint(const Point2f &point) {
    using ion::math::ArcTangent2;
    using ion::math::Length;

    // The hue is the angle around the origin.
    float hue = -ArcTangent2(point[1], point[0]).Degrees() / 360.0f;
    if (hue < 0)
        hue += 1;

    // The radius determines the saturation and value.
    const float radius = Length(Vector2f(point));
    const float t =
        Clamp((radius - kInnerRadius) / (kOuterRadius - kInnerRadius), 0, 1);
    const float sat = Lerp(t, kMinSaturation, kMaxSaturation);
    const float val = Lerp(t, kMinValue,      kMaxValue);

    return Color::FromHSV(hue, sat, val);
}

Point2f ColorRing::GetPointForColor(const Color &color) {
    using ion::math::Cosine;
    using ion::math::Sine;

    // Convert the color to HSV.
    const Vector3f hsv = color.ToHSV();

    // The hue is the angle around the center.
    const Anglef angle = Anglef::FromDegrees(hsv[0] * 360);

    // The saturation and value range from the inner radius of the disc to the
    // outer radius, so reverse-interpolate to get the radius.
    const float t = Clamp((hsv[2]    - kMinValue) /
                          (kMaxValue - kMinValue), 0, 1);
    const float radius = Lerp(t, kInnerRadius, kOuterRadius);

    // Convert to cartesian.
    return Point2f(radius * Cosine(angle), -radius * Sine(angle));
}
