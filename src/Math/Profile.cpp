#include "Math/Profile.h"

#include <algorithm>

#include <ion/base/stringutils.h>

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"

Profile::Profile() : is_open_(false), min_count_(1) {
    // min_count_ is set to 1 so the Profile is not valid.
}

Profile::Profile(const Point2f &start_point, const Point2f &end_point,
                 const PointVec &points, size_t min_count) :
    is_open_(true),
    min_count_(min_count) {
    ASSERT(min_count >= 2U);
    ASSERT(points.size() + 2 >= min_count);
    points_.push_back(start_point);
    Util::AppendVector(points, points_);
    points_.push_back(end_point);
}

Profile::Profile(const PointVec &points, size_t min_count) :
    is_open_(false),
    min_count_(min_count),
    points_(points) {
}

bool Profile::IsValid() const {
    return points_.size() >= min_count_ &&
        std::all_of(points_.begin(), points_.end(),
                    [](const Point2f &p)
                    { return (p[0] >= 0 && p[0] <= 1 &&
                              p[1] >= 0 && p[1] <= 1); });

}

Profile::PointVec Profile::GetMovablePoints() const {
    return IsOpen() ? PointVec(points_.begin() + 1,
                               points_.end()   - 1) : points_;
}

const Point2f & Profile::GetPreviousPoint(size_t index) const {
    ASSERT(index < points_.size());
    ASSERT(index > 0 || ! IsOpen());
    return index == 0 ? points_.back() : points_[index - 1];
}

const Point2f & Profile::GetNextPoint(size_t index) const {
    ASSERT(index < points_.size());
    ASSERT(index + 1 < points_.size() || ! IsOpen());
    return index + 1 == points_.size() ? points_[0] : points_[index + 1];
}

void Profile::SetPoint(size_t index, const Point2f &point) {
    ASSERTM(! (IsOpen() && (index == 0 || index + 1 == points_.size())),
            "Cannot change fixed endpoint of open Profile");
    ASSERT(index < points_.size());
    points_[index] = point;
}

void Profile::AppendPoint(const Point2f &point) {
    // If open, insert before the fixed end point.
    if (IsOpen())
        points_.insert(points_.begin() + points_.size() - 1, point);
    else
        points_.push_back(point);
}

void Profile::InsertPoint(size_t index, const Point2f &point) {
    ASSERT(index <= points_.size());
    ASSERT(! IsOpen() || (index > 0 && index < points_.size()));
    if (index == points_.size())
        points_.push_back(point);
    else
        points_.insert(points_.begin() + index, point);
}

void Profile::RemovePoint(size_t index) {
    ASSERTM(! (IsOpen() && (index == 0 || index + 1 == points_.size())),
            "Cannot remove fixed endpoint of open Profile");
    ASSERT(index < points_.size());
    ASSERT(points_.size() > min_count_);
    points_.erase(points_.begin() + index);
}

bool Profile::operator==(const Profile &p) const {
    return p.IsOpen() == IsOpen() && p.points_ == points_;
}

std::string Profile::ToString() const {
    return std::string(IsOpen() ? "OP [" : "CP [") +
        "CT=" + Util::ToString(points_.size()) +
        " MIN=" + Util::ToString(min_count_) + " <" +
        Util::JoinItems(points_, ", ") + ">]";
}
