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
