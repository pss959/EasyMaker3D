#pragma once

#include "Math/Types.h"

//! \file
//! This file defines functions that interface with the CGAL library.
//! \ingroup Math

//! Possible return codes from IsMeshValid().
enum class MeshValidityCode {
    kValid            = 0,  //!< The mesh is valid.
    kInconsistent     = 1,  //!< The mesh has inconsistent connectivity.
    kNotClosed        = 2,  //!< The mesh does not form a closed polyhedron.
    kSelfIntersecting = 3,  //!< The mesh contains self-intersections.
};

//! Returns a code indicating whether the given mesh is valid.
MeshValidityCode IsMeshValid(const TriMesh &mesh);
