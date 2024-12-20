//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Bevel.h"

#include "Util/String.h"
#include "Util/Tuning.h"

Bevel::Bevel() : max_angle(Anglef::FromDegrees(TK::kDefaultMaxBevelAngle)) {
}

Profile Bevel::CreateDefaultProfile() {
    return Profile(Profile::Type::kFixed, 2,
                   Profile::PointVec{Point2f(0, 1), Point2f(1, 0)});
}

Str Bevel::ToString() const {
    return "BEV [ " + profile.ToString() +
        " SC=" + Util::ToString(scale) +
        " MA=" + Util::ToString(max_angle) + "]";
}
