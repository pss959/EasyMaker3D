#include "Math/ColorRing.h"

#include <ion/math/angleutils.h>

#include "Base/Defaults.h"
#include "Math/Linear.h"

Color ColorRing::GetColorForPoint(const Point2f &point) {
    using ion::math::ArcTangent2;
    using ion::math::Length;

    // The hue is the angle around the origin.
    float hue = -ArcTangent2(point[1], point[0]).Degrees() / 360.0f;
    if (hue < 0)
        hue += 1;

    // The radius determines the saturation and value.
    const float radius = Length(Vector2f(point));
    const float ri = Defaults::kColorRingInnerRadius;
    const float ro = Defaults::kColorRingOuterRadius;
    const float t = Clamp((radius - ri) / (ro - ri), 0, 1);
    const float sat = Lerp(t, Defaults::kModelMinSaturation,
                           Defaults::kModelMaxSaturation);
    const float val = Lerp(t, Defaults::kModelMinValue,
                           Defaults::kModelMaxValue);

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
    const float vmin = Defaults::kModelMinValue;
    const float vmax = Defaults::kModelMaxValue;
    const float t = Clamp((hsv[2] - vmin) / (vmax   - vmin), 0, 1);
    const float radius = Lerp(t,
                              Defaults::kColorRingInnerRadius,
                              Defaults::kColorRingOuterRadius);

    // Convert to cartesian.
    return Point2f(radius * Cosine(angle), -radius * Sine(angle));
}
