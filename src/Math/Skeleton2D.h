#pragma once

#include <vector>

#include "Math/Types.h"

class Polygon;

/// 2D straight skeleton. XXXX What and why.
///
/// \ingroup Math
class Skeleton2D {
  public:
    /// Constructs the Skeleton2D for the given Polygon.
    void BuildForPolygon(const Polygon &poly);

    /// Returns the points of the skeleton.
    const std::vector<Point2f> & GetPoints() const { return points_; }

    /// Returns a vector parallel to the points vector containing the distances
    /// of each point to the contour points it bisects. This will be 0 for the
    /// points representing original vertices of the Polygon.
    const std::vector<float>   & GetDistances() const { return distances_; }

    /// Returns the bisector edges of the skeleton. Each edge is represented as
    /// a pair of point indices.
    const std::vector<size_t>  & GetEdges()  const { return edges_; }

  private:
    std::vector<Point2f> points_;
    std::vector<float>   distances_;
    std::vector<size_t>  edges_;
};
