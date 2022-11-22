#pragma once

#include <vector>

#include "Math/Types.h"

class Polygon;

/// \file
/// This file defines functions for computing straight skeletons of polygons.
///
/// \ingroup Math

/// Computes the straight skeleton of the given Polygon. XXXX
void ComputeSkeleton(const Polygon &poly, std::vector<Point2f> &vertices,
                     std::vector<size_t> &bisectors);
