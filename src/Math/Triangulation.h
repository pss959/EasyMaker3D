#pragma once

#include <vector>

#include "Math/Polygon.h"
#include "Math/Types.h"

/// \file
/// This file defines functions for triangulating polygons.
/// \ingroup Math

// ----------------------------------------------------------------------------
// 2D triangulation.
// ----------------------------------------------------------------------------

/// Triangulates the given 2D Polygon, returning a vector of indices (into the
/// Polygon's points) forming the resulting triangles.
std::vector<GIndex> TriangulatePolygon(const Polygon &poly);
