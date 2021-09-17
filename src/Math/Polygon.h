#pragma once

#include <vector>

#include "Math/Types.h"

//! A Polygon represents a planar (2D) polygon defined by an outer border of
//! vertices and any number of inner borders of vertices forming holes.
//!
//! Points are assumed to be in the proper order: border points are
//! counter-clockwise and hole points are clockwise.
//!
//! The last point in a border or hole is implicitly connected back to the first
//! to form a closed loop. There is no need to duplicate the first point at the
//! end.
class Polygon {
  public:
    //! Constructs the polygon with the given border points. The points are
    //! first cleaned up by removing any duplicate adjacent points, including
    //! the last one if it is the same as the first.
    Polygon(const std::vector<Point2f> &points);

    //! Constructs the polygon with the given points and border counts. This
    //! assumes there are no duplicate points.
    Polygon(const std::vector<Point2f> &points,
            const std::vector<size_t> border_counts);

    //! Adds a hole border with the given points to the Polygon. The points are
    //! first cleaned up by removing any duplicate adjacent points, including
    //! the last one if it is the same as the first.
    void AddHoleBorder(const std::vector<Point2f> &points);

    //! Returns a list of all points in the Polygon.
    const std::vector<Point2f> & GetPoints() const { return points_; }

    //! Returns a list of counts of points in each border. The size of this
    //! list indicates the number of borders; it should always be at least 1.
    const std::vector<size_t> & GetBorderCounts() const {
        return border_counts_;
    }

    //! Returns the outer border points. There should always be at least 3
    //! points.
    std::vector<Point2f> GetOuterBorderPoints() const {
        return std::vector<Point2f>(&points_[0], &points_[border_counts_[0]]);
    }

    //! Returns the number of hole borders. This may be zero.
    size_t GetHoleCount() const {
        return border_counts_.size() - 1;
    }

    //! Returns the points forming the border of the indexed hole. Asserts if
    //! the index is bad.
    std::vector<Point2f> GetHolePoints(size_t index) const;

    //! Computes and returns a Range2f bounding the Polygon.
    Range2f GetBoundingRect();

    //! Scales the Polygon uniformly by the given factor.
    void Scale(float scale);

    //! Translates the Polygon by the given vector.
    void Translate(const Vector2f &trans);

  private:
    //! Points are stored in order, with the counter-clockwise outer border
    //! first, followed by clockwise hole borders.
    std::vector<Point2f> points_;

    //! Number of points in each border. The size of this list indicates the
    //! number of borders; it should always be at least 1.
    std::vector<size_t>  border_counts_;

    //! Returns a vector that is the given vector of points without adjacent
    //! duplicate points (including the last one if it is the same as the
    //! first).
    static std::vector<Point2f> RemoveDups_(const std::vector<Point2f> &points);
};
