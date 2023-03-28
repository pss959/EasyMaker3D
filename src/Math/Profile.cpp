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
    min_count_(min_count),
    start_point_(start_point),
    end_point_(end_point),
    points_(points) {
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

const Point2f & Profile::GetStartPoint() const {
    ASSERT(IsOpen());
    return start_point_;
}

const Point2f & Profile::GetEndPoint() const {
    ASSERT(IsOpen());
    return end_point_;
}

Profile::PointVec Profile::GetAllPoints() const {
    if (IsOpen()) {
        PointVec points;
        points.reserve(GetTotalPointCount());
        points.push_back(start_point_);
        Util::AppendVector(points_, points);
        points.push_back(end_point_);
        return points;
    }
    else {
        return points_;
    }
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
    ASSERT(points_.size() > min_count_);
    ASSERT(index < points_.size());
    points_.erase(points_.begin() + index);
}

void Profile::AddPoints(const PointVec &points) {
        Util::AppendVector(points, points_);
    }


bool Profile::operator==(const Profile &p) const {
    if (p.IsOpen() != IsOpen())
        return false;
    if (IsOpen()) {
        return p.start_point_ == start_point_ && p.end_point_ == end_point_ &&
            p.points_ == points_;
    }
    else {
        return p.points_ == points_;
    }
}

std::string Profile::ToString() const {
    return std::string(IsOpen() ? "OP [" : "CP [") +
        "MIN: " + Util::ToString(GetMinPointCount()) + " <" +
        Util::JoinItems(GetAllPoints(), ", ") + ">]";
}
