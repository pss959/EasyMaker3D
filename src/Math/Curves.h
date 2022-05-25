#pragma once

#include <vector>

#include "Math/Types.h"

/// \file
/// This file defines functions helpful for dealing with circles and other
/// curves.
///
/// \ingroup Math

/// The CircleArc struct specifies a circular arc given by start and arc
/// angles. The direction is counterclockwise for a positive arc angle (right
/// hand rule).
///
/// \ingroup Math
struct CircleArc {
    Anglef start_angle;
    Anglef arc_angle;

    /// The default constructor defines a counterclockwise circle.
    CircleArc() : arc_angle(Anglef::FromDegrees(360)) {}

    /// Constructor taking a start angle and arc angle.
    CircleArc(const Anglef &start, const Anglef &arc) :
        start_angle(start), arc_angle(arc) {}
};

/// Returns a vector of N 2D points ranging clockwise or counterclockwise
/// around a circle with the given radius.
///
/// \ingroup Math
std::vector<Point2f> GetCirclePoints(int n, float radius, bool is_clockwise);

/// Returns a vector of N 2D points forming a counterclockwise circular arc
/// with the given radius and angles.
///
/// \ingroup Math
std::vector<Point2f> GetCircleArcPoints(int n, float radius,
                                        const CircleArc &arc);
