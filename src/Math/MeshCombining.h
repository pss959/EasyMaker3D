#pragma once

#include <vector>

#include "Math/Types.h"

/// \name Mesh combining using CGAL
///@{

/// Mesh combining operations.
enum class MeshCombiningOperation {
    kConcatenate,      ///< Just appends points and indices.
    kCSGUnion,         ///< Constructive Solid Geometry union.
    kCSGIntersection,  ///< Constructive Solid Geometry intersection.
    kCSGDifference,    ///< Constructive Solid Geometry difference.
    kConvexHull,       ///< Convex hull.
    kMinkowskiSum,     ///< Minkowski sum.
};

/// Combines the given meshes according to the operation, returning the
/// resulting mesh. Note that CSG operations require at least two meshes,
/// convex hull requires at least one mesh, and a Minkowski sum requires
/// exactly two meshes.
TriMesh CombineMeshes(const std::vector<TriMesh> &meshes,
                      MeshCombiningOperation operation);

///@}
