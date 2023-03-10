#pragma once

#include <string>
#include <vector>

#include "Math/Types.h"
#include "Util/General.h"

/// The Profile class defines a 2D profile that is used for a variety of
/// things, such as surfaces of revolution or beveling. It is defined by fixed
/// starting and ending points and a (possibly empty) series of 2D points
/// between them. All points must be in the range [0,1] in both X and Y
/// dimensions.
///
/// \ingroup Math
class Profile {
  public:
    /// The default constructor creates an empty profile with fixed starting
    /// point (0,1) and ending point (1,0).
    Profile();

    /// Constructor that sets the fixed starting and ending points.
    Profile(const Point2f &start_point, const Point2f &end_point);

    /// Returns the fixed starting point.
    const Point2f & GetStartPoint() const { return start_point_; }

    /// Returns the fixed end point.
    const Point2f & GetEndPoint()   const { return end_point_; }

    /// Adds a point to the profile.
    void AddPoint(const Point2f &p) { points_.push_back(p); }

    /// Sets all the non-fixed points..
    void SetPoints(const std::vector<Point2f> &points) { points_ = points; }

    /// Sets the indexed non-fixed point. Asserts if the index is bad.
    void SetPoint(size_t index, const Point2f &point);

    /// Inserts a point into to the profile at the given index. Asserts if the
    /// index is bad.
    void InsertPoint(size_t index, const Point2f &point);

    /// Removes the indexed point from the profile. Asserts if the index is
    /// bad.
    void RemovePoint(size_t index);

    /// Adds a set of consecutive points to the profile.
    void AddPoints(const std::vector<Point2f> &points) {
        Util::AppendVector(points, points_);
    }

    /// Returns the number of total profile points, including the start and end
    /// points.
    size_t GetPointCount() const { return points_.size() + 2; }

    /// Returns the vector of user-defined profile points (not including the
    /// start and end points).
    const std::vector<Point2f> & GetPoints() const { return points_; }

    /// Returns all points in the profile, including the fixed start and end
    /// points.
    std::vector<Point2f> GetAllPoints() const;

    /// Returns true if all interior profile points are in range and have at
    /// least the given number.
    bool IsValid(size_t min_count) const;

    /// Equality operator.
    bool operator==(const Profile &p) const {
        return p.start_point_ == start_point_ && p.end_point_ == end_point_ &&
            p.points_ == points_;
    }

    /// Inequality operator.
    bool operator!=(const Profile &p) const { return ! (p == *this); }

    /// Converts to a string to help with debugging.
    std::string ToString() const;

  private:
    Point2f              start_point_;  ///< Fixed start point.
    Point2f              end_point_;    ///< Fixed end point.
    std::vector<Point2f> points_;       ///< User-defined profile points.
};
