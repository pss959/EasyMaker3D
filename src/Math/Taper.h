//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/Dim.h"
#include "Math/Profile.h"
#include "Math/Types.h"

/// A Taper struct defines how to taper an object. It contains a Profile and
/// the axis along which to apply it.
///
/// \ingroup Math
struct Taper {
    /// Taper axis.
    Dim     axis    = Dim::kY;

    /// Taper Profile.
    Profile profile = CreateDefaultProfile();

    /// Returns true if the given Profile is valid for use in a Taper: it has
    /// to have points monotonically decreasing from 1 to 0 in Y by at least
    /// TK::kMinTaperProfileYDistance.
    static bool IsValidProfile(const Profile &prof);

    /// Equality operator.
    bool operator==(const Taper &t) const = default;

    /// Returns the default Profile used for a Taper: from (0,1) to (1,0).
    static Profile CreateDefaultProfile();

    /// Converts to a string to help with debugging.
    Str ToString() const;
};
