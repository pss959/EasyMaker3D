#include "Math/Curves.h"

#include <ion/math/angleutils.h>

#include "Math/Linear.h"

std::vector<Point2f> GetCirclePoints(int n, float radius, bool is_clockwise) {
    std::vector<Point2f> points;
    points.resize(n);
    for (int i = 0; i < n; ++i) {
        const Anglef angle = Anglef::FromDegrees(i * 360.f / n);
        points[i].Set(radius * ion::math::Cosine(angle),
                      radius * ion::math::Sine(angle));
        if (is_clockwise)
            points[i][1] = -points[i][1];
    }

    // Make sure the bounds of the circle are as close as possible to the
    // desired diameter.
    Range2f bounds;
    for (const auto &p: points)
        bounds.ExtendByPoint(p);
    const Vector2f size        = bounds.GetSize();
    const float    target_size = 2 * radius;
    if (! AreClose(size[0], target_size) || ! AreClose(size[1], target_size)) {
        const float sx = target_size / size[0];
        const float sy = target_size / size[1];
        for (auto &p: points) {
            p[0] *= sx;
            p[1] *= sy;
        }
    }

    return points;
}

std::vector<Point2f> GetCircleArcPoints(int n, float radius,
                                        const CircleArc &arc) {
    std::vector<Point2f> points;
    points.resize(n);
    const Anglef delta_angle = arc.arc_angle / (n - 1.f);
    for (int i = 0; i < n; ++i) {
        const Anglef angle = arc.start_angle + i * delta_angle;
        points[i].Set(radius * ion::math::Cosine(angle),
                      radius * ion::math::Sine(angle));
    }
    return points;
}
