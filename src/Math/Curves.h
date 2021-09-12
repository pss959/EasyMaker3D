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
