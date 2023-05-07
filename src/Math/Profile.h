#pragma once

#include <string>
#include <vector>

#include "Math/Types.h"

/// The Profile class defines a 2D profile that is used for a variety of
/// things, such as surfaces of revolution, extrusions, or beveling. All points
/// in a Profile must be in the range [0,1] in both X and Y dimensions.
///
/// There are three types of Profile:
///   - A \em Fixed Profile has fixed starting and ending points and any number
///     of movable points between them.
///   - A \em Closed Profile has 3 or more movable points and the last point is
///     implicitly connected to the first point to form a loop.
///   - An \em Open Profile has two or more movable points but it is not
///   - implicitly closed.
///
/// The constructor used to create the Profile determines its type. Each
/// constructor takes a minimum point count; the Profile is not allowed to have
/// fewer points than this minimum. Note that the default constructor creates
/// an invalid, undefined Profile.
///
/// \ingroup Math
class Profile {
  public:
    /// Profile types.
    enum class Type {
        kFixed,   ///< Fixed starting and ending points.
        kClosed,  ///< No fixed points, implicitly closed to form a loop.
        kOpen,    ///< No fixed points, ends are left unconnected.
    };

    /// Convenience typedef.
    typedef std::vector<Point2f> PointVec;

    /// The default constructor creates an invalid, undefined Profile.
    Profile();

    /// Constructor that creates a Profile of the given \p type, minimum number
    /// of points, and \p points. For a Fixed Profile, there must be at least 2
    /// points in the vector (the fixed starting and ending points). For a
    /// Closed Profile, there must be at least 3 points. For an Open Profile,
    /// there must be at least 2 points.
    Profile(Type type, size_t min_count, const PointVec &points);

    /// Convenience that creates a Fixed Profile with the given minimum number
    /// of points, fixed start and end points, and internal points.
    static Profile CreateFixedProfile(const Point2f &start_point,
                                      const Point2f &end_point,
                                      size_t min_count, const PointVec &points);

    /// \name Query
    ///@{

    /// Returns true if all points are in range and there are at least the
    /// required number.
    bool IsValid() const;

    /// Returns the Profile type.
    Type GetType() const { return type_; }

    /// Returns the minimum number of points for the Profile.
    size_t GetMinPointCount() const { return min_count_; }

    /// Returns the number of Profile points.
    size_t GetPointCount() const { return points_.size(); }

    /// Returns true if the given index refers to a fixed point in a Fixed
    /// Profile.
    bool IsFixedPoint(size_t index) const;

    /// Returns the number of movable Profile points.
    size_t GetMovablePointCount() const {
        return points_.size() - (type_ == Type::kFixed ? 2 : 0);
    }

    /// Returns the vector of Profile points.
    const PointVec & GetPoints() const { return points_; }

    /// Returns a vector containing only movable points. For a Fixed Profile,
    /// this is all points between the fixed end points. For Closed and Open
    /// Profiles, this is the same as GetPoints().
    PointVec GetMovablePoints() const;

    /// Returns the location of the point before the indexed point. If the
    /// Profile is not Closed, this asserts if the index is 0. If it is Closed,
    /// it returns the last point if the index is 0.
    const Point2f & GetPreviousPoint(size_t index) const;

    /// Returns the location of the point after the indexed point. If the
    /// Profile is not Closed, this asserts if the index is for the last
    /// point. If it is Closed, it returns the first point if the index is for
    /// the last point.
    const Point2f & GetNextPoint(size_t index) const;

    ///@}

    /// \name Modification
    ///@{

    /// Changes the position of the indexed point. Asserts if the index is bad
    /// or if the index refers to a fixed point.
    void SetPoint(size_t index, const Point2f &point);

    /// Appends a movable point to to the profile. Note that for a Fixed
    /// Profile this inserts the point just before the fixed end point.
    void AppendPoint(const Point2f &point);

    /// Inserts a point into to the profile at the given index. Asserts if the
    /// index is bad.
    void InsertPoint(size_t index, const Point2f &point);

    /// Removes the indexed point from the profile. Asserts if the index is bad
    /// or refers to a fixed point, or if the remaining number of points is
    /// below the minimum.
    void RemovePoint(size_t index);

    ///@}

    /// Equality operator.
    bool operator==(const Profile &p) const;

    /// Inequality operator.
    bool operator!=(const Profile &p) const { return ! (p == *this); }

    /// Converts to a string to help with debugging.
    std::string ToString() const;

  private:
    Type     type_;
    size_t   min_count_;  ///< Minimum number of points.
    PointVec points_;     ///< All points in the Profile.
};
