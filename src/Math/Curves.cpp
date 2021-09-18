#include "Math/Curves.h"

#include <ion/math/angleutils.h>

std::vector<Point2f> GetCirclePoints(int n, float radius) {
    std::vector<Point2f> points;
    points.resize(n);
    for (int i = 0; i < n; ++i) {
        const Anglef angle = Anglef::FromDegrees(i * 360.f / n);
        points[i].Set( radius  * ion::math::Cosine(angle),
                       -radius * ion::math::Sine(angle));
    }
    return points;
}

std::vector<Point2f> GetCircleArcPoints(int n, float radius,
                                        const Anglef &start_angle,
                                        const Anglef &arc_angle) {
    std::vector<Point2f> points;
    points.resize(n);
    const Anglef delta_angle = arc_angle / (n - 1.f);
    for (int i = 0; i < n; ++i) {
        const Anglef angle = start_angle + i * delta_angle;
        points[i].Set(radius * ion::math::Cosine(angle),
                      radius * ion::math::Sine(angle));
    }
    return points;
}
