#pragma once

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
