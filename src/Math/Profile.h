#pragma once

#include <string>
#include <vector>

#include "Math/Types.h"

/// The Profile class defines a 2D profile that is used for a variety of
/// things, such as surfaces of revolution, extrusions, or beveling. All points
/// in a Profile must be in the range [0,1] in both X and Y dimensions.
///
/// There are two types of Profile:
///   - An \em open Profile has fixed starting and ending points and any number
///     of movable points between them.
///   - A \em closed Profile has no fixed points; all points are considered
///     movable, and the last point is implicitly connected to the first point.
///
/// The constructor used to create the Profile defines whether it is open or
/// closed. Each of these constructors takes a minimum (movable) point count.
/// The Profile is not allowed to have fewer movable points than this minimum.
/// Note that the default constructor creates an invalid, undefined
/// Profile.
///
/// \ingroup Math
class Profile {
  public:
    /// Convenience typedef.
    typedef std::vector<Point2f> PointVec;

    /// The default constructor creates an invalid, undefined Profile.
    Profile();

    /// Constructor that creates an open profile using the given starting and
    /// ending points, with no movable points and a minimum of 0 movable
    /// points.
    Profile(const Point2f &start_point, const Point2f &end_point) :
        Profile(start_point, end_point, PointVec(), 0) {}

    /// Constructor that creates an open profile using the given starting and
    /// ending points, with the given vector of initial movable points and the
    /// given minimum number of movable points.
    Profile(const Point2f &start_point, const Point2f &end_point,
            const PointVec &points, size_t min_count);

    /// Constructor that creates a closed profile with the given (movable)
    /// points and minimum number of movable points.
    Profile(const PointVec &points, size_t min_count);

    /// \name Query
    ///@{

    /// Returns true if all movable points are in range and have at least the
    /// required number.
    bool IsValid() const;

    /// Returns true if the Profile is open.
    bool IsOpen() const { return is_open_; }

    /// Returns the minimum number of movable Profile points passed to the
    /// constructor.
    size_t GetMinPointCount() const { return min_count_; }

    /// Returns the fixed starting point for an open Profile. Asserts if the
    /// Profile is not open.
    const Point2f & GetStartPoint() const;

    /// Returns the fixed end point for an open Profile. Asserts if the Profile
    /// is not open.
    const Point2f & GetEndPoint() const;

    /// Returns the number of movable Profile points.
    size_t GetPointCount() const { return points_.size(); }

    /// Returns the total number of Profile points, including the fixed start
    /// and end points for an open Profile.
    size_t GetTotalPointCount() const {
        return GetPointCount() + (IsOpen() ? 2 : 0);
    }

    /// Returns the vector of user-defined profile points (not including the
    /// start and end points).
    const PointVec & GetPoints() const { return points_; }

    /// Returns all points in the profile, including the fixed start and end
    /// points.
    PointVec GetAllPoints() const;

    /// Returns the location of the point before the indexed movable point. If
    /// the Profile is open, this may return the fixed start point. If it is
    /// closed, this may wrap around to the last movable point. Asserts if the
    /// index is bad.
    const Point2f & GetPreviousPoint(size_t index) const;

    /// Returns the location of the point after the indexed movable point. If
    /// the Profile is open, this may return the fixed end point. If it is
    /// closed, this may wrap around to the first movable point. Asserts if the
    /// index is bad.
    const Point2f & GetNextPoint(size_t index) const;

    ///@}

    /// \name Modification
    ///@{

    /// Adds a movable point to the profile.
    void AddPoint(const Point2f &p) { points_.push_back(p); }

    /// Sets all the movable points..
    void SetPoints(const PointVec &points) { points_ = points; }

    /// Sets the indexed movable point. Asserts if the index is bad.
    void SetPoint(size_t index, const Point2f &point);

    /// Inserts a movable point into to the profile at the given index. Asserts
    /// if the index is bad.
    void InsertPoint(size_t index, const Point2f &point);

    /// Removes the indexed movable point from the profile. Asserts if the
    /// index is bad.
    void RemovePoint(size_t index);

    /// Adds a set of consecutive points to the profile.
    void AddPoints(const PointVec &points);

    ///@}

    /// Equality operator.
    bool operator==(const Profile &p) const;

    /// Inequality operator.
    bool operator!=(const Profile &p) const { return ! (p == *this); }

    /// Converts to a string to help with debugging.
    std::string ToString() const;

  private:
    bool     is_open_;
    size_t   min_count_;          ///< Minimum number of movable points.
    Point2f  start_point_{0, 0};  ///< Fixed start point (if open).
    Point2f  end_point_{0, 0};    ///< Fixed end point (if open).
    PointVec points_;             ///< Movable points.
};
