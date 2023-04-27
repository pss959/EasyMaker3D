#pragma once

#include <vector>

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

///@}
