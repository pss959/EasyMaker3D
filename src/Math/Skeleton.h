#pragma once

class Polygon;

/// \file
/// This file defines functions for computing straight skeletons of polygons.
///
/// \ingroup Math

/// Computes the straight skeleton of the given Polygon.
void ComputeSkeleton(const Polygon &poly);
