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
///     movable, and the last point is implicitly connected to the first point
///     to form a loop.
///
/// The constructor used to create the Profile determines whether it is open or
/// closed. Each constructor takes a minimum total point count. The Profile is
/// not allowed to have fewer points than this minimum. Note that the default
/// constructor creates an invalid, undefined Profile.
///
/// \ingroup Math
class Profile {
  public:
    /// Convenience typedef.
    typedef std::vector<Point2f> PointVec;

    /// The default constructor creates an invalid, undefined Profile.
    Profile();

    /// Constructor that creates an open profile using the given starting and
    /// ending points, with no movable points and a minimum of 2 points.
    Profile(const Point2f &start_point, const Point2f &end_point) :
        Profile(start_point, end_point, PointVec(), 2) {}

    /// Constructor that creates an open profile using the given starting and
    /// ending points, with the given vector of initial movable points and the
    /// given minimum number of points (>= 2).
    Profile(const Point2f &start_point, const Point2f &end_point,
            const PointVec &points, size_t min_count);

    /// Constructor that creates a closed profile with the given (movable)
    /// points and minimum number of movable points.
    Profile(const PointVec &points, size_t min_count);

    /// \name Query
    ///@{

    /// Returns true if all points are in range and there are at least the
    /// required number.
    bool IsValid() const;

    /// Returns true if the Profile is open.
    bool IsOpen() const { return is_open_; }

    /// Returns the minimum number of points for the Profile.
    size_t GetMinPointCount() const { return min_count_; }

    /// Returns the number of Profile points.
    size_t GetPointCount() const { return points_.size(); }

    /// Returns the vector of Profile points.
    const PointVec & GetPoints() const { return points_; }

    /// Returns a vector containing only movable points. For an open Profile,
    /// this is all points between the fixed end points. For a closed Profile,
    /// this is the same as GetPoints().
    PointVec GetMovablePoints() const;

    /// Returns the location of the point before the indexed point. If the
    /// Profile is open, this asserts if the index is 0. If it is closed, it
    /// returns the last point if the index is 0.
    const Point2f & GetPreviousPoint(size_t index) const;

    /// Returns the location of the point after the indexed point. If the
    /// Profile is open, this asserts if the index is for the last point. If it
    /// is closed, it returns the first point if the index is for the last
    /// point.
    const Point2f & GetNextPoint(size_t index) const;

    ///@}

    /// \name Modification
    ///@{

    /// Changes the position of the indexed point. Asserts if the index is bad
    /// or if the Profile is open and the point is fixed.
    void SetPoint(size_t index, const Point2f &point);

    /// Appends a movable point to to the profile. Note that for an open
    /// Profile this inserts the point just before the fixed end point.
    void AppendPoint(const Point2f &point);

    /// Inserts a point into to the profile at the given index. Asserts if the
    /// index is bad.
    void InsertPoint(size_t index, const Point2f &point);

    /// Removes the indexed point from the profile. Asserts if the index is
    /// bad or refers to a fixed point in an open Profile.
    void RemovePoint(size_t index);

    ///@}

    /// Equality operator.
    bool operator==(const Profile &p) const;

    /// Inequality operator.
    bool operator!=(const Profile &p) const { return ! (p == *this); }

    /// Converts to a string to help with debugging.
    std::string ToString() const;

  private:
    bool     is_open_;    ///< True if this is an open Profile.
    size_t   min_count_;  ///< Minimum number of points.
    PointVec points_;     ///< All points in the Profile.
};
