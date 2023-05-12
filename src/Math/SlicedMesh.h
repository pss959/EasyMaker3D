#pragma once

#include <vector>

#include "Math/Types.h"

/// A SlicedMesh represents a TriMesh that has been sliced by some number of
/// planes parallel to a given direction. It saves information that can be used
/// to avoid extra computations for Model types that need to slice during mesh
/// construction.
struct SlicedMesh {
    TriMesh  mesh;
    Vector3f dir;
    Range1f  range;
};

// XXXX Move all of this?

/// Slices a mesh (in place) by a series of Planes parallel to the given
/// direction. The \p fractions vector contains the relative distances in the
/// range (0,1) of the planes relative to the min/max distances of the mesh
/// along the direction vector.
SlicedMesh NSliceMesh(const TriMesh &mesh, const Vector3f &dir,
                      const std::vector<float> &fractions);
