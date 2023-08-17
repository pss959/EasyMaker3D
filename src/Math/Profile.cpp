#include "Math/Profile.h"

#include <algorithm>

#include <ion/base/stringutils.h>

#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/String.h"

Profile::Profile() : type_(Type::kOpen), min_count_(0) {
    // min_count_ is set to 0 so the Profile is not valid.
}

Profile::Profile(Type type, size_t min_count, const PointVec &points) :
    type_(type),
    min_count_(min_count),
    points_(points) {
}

Profile Profile::CreateFixedProfile(const Point2f &start_point,
                                    const Point2f &end_point,
                                    size_t min_count, const PointVec &points) {
    PointVec all_points;
    all_points.reserve(2 + points.size());
    all_points.push_back(start_point);
    Util::AppendVector(points, all_points);
    all_points.push_back(end_point);
    return Profile(Profile::Type::kFixed, min_count, all_points);
}

bool Profile::IsValid() const {
    // Make sure the minimum number of points is valid for the type.
    const size_t min = type_ == Type::kClosed ? 3U : 2U;
    if (min_count_ < min)
        return false;

    // Make sure there are enough points and all of them are in range.
    return points_.size() >= min_count_ &&
        std::all_of(points_.begin(), points_.end(),
                    [](const Point2f &p)
                    { return (p[0] >= 0 && p[0] <= 1 &&
                              p[1] >= 0 && p[1] <= 1); });

}

bool Profile::IsFixedPoint(size_t index) const {
    ASSERT(index < points_.size());
    return type_ == Type::kFixed && (index == 0 || index + 1 == points_.size());
}

Profile::PointVec Profile::GetMovablePoints() const {
    if (type_ == Type::kFixed)
        return PointVec(points_.begin() + 1, points_.end()   - 1);
    else
        return points_;
}

const Point2f & Profile::GetPreviousPoint(size_t index) const {
    ASSERT(index < points_.size());
    ASSERT(index > 0 || type_ == Type::kClosed);
    return index == 0 ? points_.back() : points_[index - 1];
}

const Point2f & Profile::GetNextPoint(size_t index) const {
    ASSERT(index < points_.size());
    ASSERT(index + 1 < points_.size() || type_ == Type::kClosed);
    return index + 1 == points_.size() ? points_[0] : points_[index + 1];
}

void Profile::SetPoint(size_t index, const Point2f &point) {
    // Cannot change a fixed point.
    ASSERT(! IsFixedPoint(index));
    points_[index] = point;
}

void Profile::AppendPoint(const Point2f &point) {
    // If Fixed, insert before the fixed end point.
    if (type_ == Type::kFixed)
        points_.insert(points_.begin() + points_.size() - 1, point);
    else
        points_.push_back(point);
}

void Profile::InsertPoint(size_t index, const Point2f &point) {
    ASSERT(index <= points_.size());
    ASSERT(type_ == Type::kClosed || (index > 0 && index < points_.size()));
    if (index == points_.size())
        points_.push_back(point);
    else
        points_.insert(points_.begin() + index, point);
}

void Profile::RemovePoint(size_t index) {
    ASSERT(! IsFixedPoint(index));
    ASSERT(points_.size() > min_count_);
    points_.erase(points_.begin() + index);
}

Str Profile::ToString() const {
    return Str("PROF[") + Util::EnumName(type_) +
        " MN=" + Util::ToString(min_count_) + " <" +
        " CT=" + Util::ToString(points_.size()) +
        Util::JoinItems(points_, ", ") + ">]";
}
