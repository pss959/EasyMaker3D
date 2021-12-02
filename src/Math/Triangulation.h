#pragma once

#include <vector>

#include "Math/Polygon.h"

/// \file
/// This file defines functions 
/// \ingroup Math

// ----------------------------------------------------------------------------
// 2D triangulation.
// ----------------------------------------------------------------------------

/// Triangulates the given 2D Polygon, returning a vector of indices (into the
/// Polygon's points) forming the resulting triangles.
std::vector<size_t> TriangulatePolygon(const Polygon &poly);

///@}
