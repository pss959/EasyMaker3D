#pragma once

#include <vector>

#include "Math/SlicedMesh.h"
#include "Math/Types.h"

/// \file
/// This file defines operations for dividing TriMesh instances using CGAL
///
/// \ingroup Math

/// \name Mesh dividing using CGAL
///@{

/// Clips a mesh (in place) by a Plane, returning the resulting mesh.
TriMesh ClipMesh(const TriMesh &mesh, const Plane &plane);

/// Splits a mesh (in place) by a Plane, returning the resulting mesh.
TriMesh SplitMesh(const TriMesh &mesh, const Plane &plane);

/// Divides a mesh evenly into the given number (>= 1) of slices along the
/// given direction vector. Returns a SlicedMesh representing the result.
SlicedMesh SliceMesh(const TriMesh &mesh, const Vector3f &dir,
                     size_t num_slices);

/// Slices a mesh (in place) by a series of Planes parallel to the given
/// direction. The \p fractions vector contains the relative distances in the
/// range (0,1) of the planes relative to the min/max distances of the mesh
/// along the direction vector.
SlicedMesh SliceMesh(const TriMesh &mesh, const Vector3f &dir,
                     const std::vector<float> &fractions);

///@}
