#include "Math/Polygon.h"

#include "Util/Assert.h"
#include "Util/General.h"

Polygon::Polygon(const std::vector<Point2f> &points) {
    ASSERT(points.size() >= 3U);
    points_ = RemoveDups_(points);
    border_counts_.push_back(points_.size());
}

Polygon::Polygon(const std::vector<Point2f> &points,
                 const std::vector<size_t> border_counts) {
    ASSERT(points.size() >= 3U);
    points_ = points;
    if (! border_counts.empty())
        border_counts_ = border_counts;
    else
        border_counts_.push_back(points_.size());
}

void Polygon::AddHoleBorder(const std::vector<Point2f> &points) {
    ASSERT(points.size() >= 3U);
    std::vector<Point2f> clean_points = RemoveDups_(points);
    Util::AppendVector(clean_points, points_);
    border_counts_.push_back(clean_points.size());
}

std::vector<Point2f> Polygon::GetHolePoints(size_t index) const {
    ASSERT(index < GetHoleCount());
    size_t start = 0;
    for (size_t i = 0; i <= index; ++i)
        start += border_counts_[i];
    return std::vector<Point2f>(&points_[start],
                                &points_[start + border_counts_[index + 1]]);
}

Range2f Polygon::GetBoundingRect() const {
    Range2f bounds;
    for (const auto &p: GetOuterBorderPoints())
        bounds.ExtendByPoint(p);
    return bounds;
}

void Polygon::Scale(float scale) {
    for (auto &p: points_)
        p *= scale;
}

void Polygon::Translate(const Vector2f &trans) {
    for (auto &p: points_)
        p += trans;
}

std::vector<Point2f> Polygon::RemoveDups_(const std::vector<Point2f> &points) {
    std::vector<Point2f> clean_points;
    for (auto &p: points) {
        if (clean_points.empty() || p != clean_points.back())
            clean_points.push_back(p);
    }
    // Remove the last point if it is the same as the first.
    if (clean_points.back() == clean_points[0])
        clean_points.pop_back();
    return clean_points;
}

void Polygon::Dump(const std::string &when) const {
    std::cout << "=== " << when << ": Polygon with "
              << points_.size() << " points:\n";
    for (size_t i = 0; i < points_.size(); ++i)
        std::cout << "  [" << i << "] " << points_[i] << "\n";
    std::cout << "=== ... Outer border size: " << border_counts_[0] << "\n";
    if (border_counts_.size() > 0) {
        std::cout << "=== ... Hole border sizes:";
        for (size_t i = 1; i < border_counts_.size(); ++i)
            std::cout << " " << border_counts_[i];
        std::cerr << "\n";
    }
}
