#pragma once

#include <vector>

#include "Math/SlicedMesh.h"

/// \file
/// Mesh Slicing functions
///
/// \ingroup Math

/// Slices a mesh (in place) by a series of Planes perpendicular to the axis in
/// the given dimension. The \p fractions vector contains the relative
/// distances in the range (0,1) of the planes relative to the min/max
/// distances of the mesh along the axis.
SlicedMesh SliceMesh(const TriMesh &mesh, Dim dim,
                     const std::vector<float> &fractions);

/// Divides a mesh evenly into the given number (>= 1) of slices along the axis
/// in the given dimension. Returns a SlicedMesh representing the result.
SlicedMesh SliceMesh(const TriMesh &mesh, Dim dim, size_t num_slices);
