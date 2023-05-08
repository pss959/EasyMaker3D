#pragma once

#include <string>

#include "Enums/Axis.h"
#include "Math/Profile.h"
#include "Math/Types.h"

/// A Taper struct defines how to taper an object. It contains a Profile and
/// the axis along which to apply it.
///
/// \ingroup Math
struct Taper {
    /// Taper axis.
    Axis    axis    = Axis::kY;

    /// Taper Profile.
    Profile profile = CreateDefaultProfile();

    /// XXXX IsValid() - has to have profile with points decreasing from 1 to 0
    /// in Y, separated at least by TK::kSomething.
    static bool IsValidProfile(const Profile &prof);

    /// XXXX Add func to get X for Y in [0,1].
    float GetX(float y) const;

    /// Equality operator.
    bool operator==(const Taper &t) const {
        return t.axis == axis && t.profile == profile;
    }

    /// Inequality operator.
    bool operator!=(const Taper &t) const { return ! (t == *this); }

    /// Returns the default Profile used for a Taper: from (0,1) to (1,0).
    static Profile CreateDefaultProfile();

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};
