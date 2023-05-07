#include "Math/Bevel.h"

#include "Util/String.h"
#include "Util/Tuning.h"

Bevel::Bevel() : max_angle(Anglef::FromDegrees(TK::kDefaultMaxBevelAngle)) {
}

Profile Bevel::CreateDefaultProfile() {
    return Profile(Profile::Type::kFixed, 2,
                   Profile::PointVec{Point2f(0, 1), Point2f(1, 0)});
}

std::string Bevel::ToString() const {
    return "BEV [ " + profile.ToString() +
        " SC=" + Util::ToString(scale) +
        " MA=" + Util::ToString(max_angle) + "]";
}
