#pragma once

#include <vector>

#include "Math/Polygon.h"
#include "Math/Types.h"

//! \file
//! This file defines functions that interface with the CGAL library.
//! \ingroup Math

// ----------------------------------------------------------------------------
// 2D triangulation.
// ----------------------------------------------------------------------------

//! Triangulates the given 2D Polygon, returning a vector of indices (into the
//! Polygon's points) forming the resulting triangles.
std::vector<size_t> TriangulatePolygon(const Polygon &poly);

// ----------------------------------------------------------------------------
// Mesh validation.
// ----------------------------------------------------------------------------

//! \name Mesh Validation
//!@{

//! Possible return codes from IsMeshValid().
enum class MeshValidityCode {
    kValid,             //!< The mesh is valid.
    kInconsistent,      //!< The mesh has inconsistent connectivity.
    kNotClosed,         //!< The mesh does not form a closed polyhedron.
    kSelfIntersecting,  //!< The mesh contains self-intersections.
};

//! Returns a code indicating whether the given mesh is valid.
MeshValidityCode IsMeshValid(const TriMesh &mesh);

//!@}

// ----------------------------------------------------------------------------
// Mesh combining.
// ----------------------------------------------------------------------------

//! \name Mesh Combining
//!@{

//! Mesh combining operations.
enum class MeshCombiningOperation {
    kCSGUnion,         //!< Constructive Solid Geometry union.
    kCSGIntersection,  //!< Constructive Solid Geometry intersection.
    kCSGDifference,    //!< Constructive Solid Geometry difference.
    kConvexHull,       //!< Convex hull.
    kMinkowskiSum,     //!< Minkowski sum.
};

//! Combines the given meshes according to the operation, returning the
//! resulting mesh. Note that CSG operations require at least two meshes,
//! convex hull requires at least one mesh, and a Minkowski sum requires
//! exactly two meshes.
TriMesh CombineMeshes(const std::vector<TriMesh> &meshes,
                      MeshCombiningOperation operation);

//!@}

// ----------------------------------------------------------------------------
// Other mesh operations.
// ----------------------------------------------------------------------------

//! \name Other Mesh Operations
//!@{

//! Clips a mesh by a plane, returning the resulting mesh.
TriMesh ClipMesh(const TriMesh &mesh, const Plane &plane);

//!@}
