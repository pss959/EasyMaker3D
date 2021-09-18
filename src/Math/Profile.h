#pragma once

#include <vector>

#include "Math/Types.h"

//! The Profile class defines a 2D profile that is used for a variety of
//! things, such as surfaces of revolution or beveling. It is defined by fixed
//! starting and ending points and a (possibly empty) series of 2D points
//! between them. All points must be in the range [0,1] in both X and Y
//! dimensions.
class Profile {
  public:
    //! The default constructor creates an empty profile with fixed starting
    //! point (0,0) and ending point (1,1).
    Profile();

    //! Constructor that sets the fixed starting and ending points.
    Profile(const Point2f &start_point, const Point2f &end_point);

    //! Returns the fixed starting point.
    const Point2f & GetStartPoint() const { return start_point_; }

    //! Returns the fixed end point.
    const Point2f & GetEndPoint()   const { return end_point_; }

    //! Adds a point to the profile.
    void AddPoint(const Point2f &p) { points_.push_back(p); }

    //! Returns the number of total profile points, including the start and end
    //! points.
    size_t GetPointCount() const { return points_.size() + 2; }

    //! Returns the vector of user-defined profile points (not including the
    //! start and end points).
    const std::vector<Point2f> & GetPoints() const { return points_; }

    //! Returns all points in the profile, including the fixed start and end
    //! points.
    std::vector<Point2f> GetAllPoints() const;

    //! Returns true if all interior profile points are in range and have at
    //! least the given number.
    bool IsValid(size_t min_count) const;

  private:
    Point2f              start_point_;  //!< Fixed start point.
    Point2f              end_point_;    //!< Fixed end point.
    std::vector<Point2f> points_;       //!< User-defined profile points.
};
