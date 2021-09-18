#pragma once

#include <vector>

#include "Math/Types.h"

//! \file
//! This file defines functions helpful for dealing with circles and other
//! curves.
//! \ingroup Math

//! Returns a vector of N 2D points ranging counterclockwise around a circle
//! with the given radius.
std::vector<Point2f> GetCirclePoints(int n, float radius);

//! Returns a vector of N 2D points forming a counterclockwise circular arc
//! with the given radius and angles.
std::vector<Point2f> GetCircleArcPoints(int n, float radius,
                                        const Anglef &start_angle,
                                        const Anglef &arc_angle);
