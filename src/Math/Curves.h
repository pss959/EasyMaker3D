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

//! Returns a vector of N 2D points forming a circular arc with the given
//! radius and angles in the direction specified by is_clockwise.
std::vector<Point2f> GetCircleArcPoints(int n, float radius,
                                        const Anglef &start_angle,
                                        const Anglef &arc_angle,
                                        bool is_clockwise);
