#pragma once

#include <vector>

#include "Math/SlicedMesh.h"

/// Slices a mesh (in place) by a series of Planes perpendicular to the given
/// axis. The \p fractions vector contains the relative distances in the range
/// (0,1) of the planes relative to the min/max distances of the mesh along the
/// axis.
SlicedMesh SliceMesh(const TriMesh &mesh, Axis axis,
                     const std::vector<float> &fractions);

/// Divides a mesh evenly into the given number (>= 1) of slices along the
/// given axis. Returns a SlicedMesh representing the result.
SlicedMesh SliceMesh(const TriMesh &mesh, Axis axis, size_t num_slices);
