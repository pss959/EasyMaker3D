#pragma once

#include <vector>

#include "Math/Types.h"

struct PolyMesh;

/// 3D straight skeleton. XXXX What and why.
///
/// \ingroup Math
class Skeleton3D {
  public:
    /// Constructs the Skeleton for the given PolyMesh.
    void BuildForPolyMesh(const PolyMesh &mesh);

    /// Returns the points of the skeleton.
    const std::vector<Point3f> & GetPoints() const { return points_; }

    /// Returns the bisector edges of the skeleton. Each edge is represented as
    /// a pair of point indices.
    const std::vector<size_t>  & GetEdges()  const { return edges_; }

  private:
    std::vector<Point3f> points_;
    std::vector<size_t>  edges_;

    /// Adds one face of a PolyMesh.
    void AddFace_(const std::vector<Point3f> &points,
                  const std::vector<size_t> &border_counts,
                  const Vector3f &normal);
};
