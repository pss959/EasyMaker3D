#include "Math/Profile.h"

#include <algorithm>

Profile::Profile() : start_point_(0, 0), end_point_(1, 1) {
}

Profile::Profile(const Point2f &start_point, const Point2f &end_point) :
    start_point_(start_point), end_point_(end_point) {
}

std::vector<Point2f> Profile::GetAllPoints() const {
    std::vector<Point2f> points;
    points.reserve(points_.size() + 2);
    points.push_back(start_point_);
    points.insert(points.end(), points_.begin(), points_.end());
    points.push_back(end_point_);
    return points;
}

bool Profile::IsValid(size_t min_count) const {
    return points_.size() >= min_count &&
        std::all_of(points_.begin(), points_.end(),
                    [](const Point2f &p)
                    { return (p[0] >= 0 && p[0] <= 1 &&
                              p[1] >= 0 && p[1] <= 1); });
}
