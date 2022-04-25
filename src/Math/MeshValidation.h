#pragma once

#include <vector>

#include "Math/Types.h"

/// \name Mesh Validation
///
/// \ingroup Math
///@{

/// Possible return codes from IsMeshValid().
enum class MeshValidityCode {
    kValid,             ///< The mesh is valid.
    kInconsistent,      ///< The mesh has inconsistent connectivity.
    kNotClosed,         ///< The mesh does not form a closed polyhedron.
    kSelfIntersecting,  ///< The mesh contains self-intersections.
};

/// Returns a code indicating whether the given mesh is valid.
MeshValidityCode IsMeshValid(const TriMesh &mesh);

///@}
