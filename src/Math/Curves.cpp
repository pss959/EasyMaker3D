#include "Math/Curves.h"

#include <ion/math/angleutils.h>

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
