#include "Math/Profile.h"

#include <algorithm>

#include <ion/base/stringutils.h>

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"

Profile::Profile() : start_point_(0, 1), end_point_(1, 0) {
}

Profile::Profile(const Point2f &start_point, const Point2f &end_point) :
    start_point_(start_point), end_point_(end_point) {
}

void Profile::SetPoint(size_t index, const Point2f &point) {
    ASSERT(index < points_.size());
    points_[index] = point;
}

void Profile::InsertPoint(size_t index, const Point2f &point) {
    ASSERT(index <= points_.size());
    if (index == points_.size())
        points_.push_back(point);
    else
        points_.insert(points_.begin() + index, point);
}

void Profile::RemovePoint(size_t index) {
    ASSERT(index < points_.size());
    points_.erase(points_.begin() + index);
}

std::vector<Point2f> Profile::GetAllPoints() const {
    std::vector<Point2f> points;
    points.reserve(points_.size() + 2);
    points.push_back(start_point_);
    Util::AppendVector(points_, points);
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

std::string Profile::ToString() const {
    std::string s = "PR [<" + Util::ToString(GetStartPoint()) + ">, " +
        Util::JoinItems(GetPoints(), ", ");
    if (! GetPoints().empty())
        s += ", ";
    s += "<" + Util::ToString(GetEndPoint()) + ">]";
    return s;
}
