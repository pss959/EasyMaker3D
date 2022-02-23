#pragma once

#include <string>

#include "Defaults.h"
#include "Math/Profile.h"
#include "Math/Types.h"

/// A Bevel defines how to bevel edges of an object. It contains a Profile, a
/// scale factor, and the maximum edge angle used to determine which edges to
/// bevel.
///
/// The bevel Profile is applied relative to (1,1), which is where an edge is
/// assumed to lie. The fixed points for the Profile must be (0,1) at the start
/// and (1,0) at the end. Without any other points, the Profile is a straight
/// line joining those 2 points, creating a simple bevel.
struct Bevel {
    /// Profile applied to each beveled edge.
    Profile profile = CreateDefaultProfile();

    /// Scale factor applied uniformly to the profile points. The default is 1.
    float   scale = 1;

    /// Maximum angle for an edge to have a bevel profile applied to it. Any
    /// edge whose faces form an angle larger than this will be left alone. The
    /// default is 120 degrees.
    Anglef  max_angle = Anglef::FromDegrees(Defaults::kMaxBevelAngle);

    /// Returns the default Profile used for a Bevel: just two fixed points at
    /// (0,1) and (1,0), forming a straight line.
    static Profile CreateDefaultProfile() {
        return Profile(Point2f(0, 1), Point2f(1, 0));
    }

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};
