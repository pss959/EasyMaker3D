#include "Math/Taper.h"

#include "Util/Enum.h"
#include "Util/String.h"

Profile Taper::CreateDefaultProfile() {
    return Profile(Profile::Type::kOpen, 2,
                   Profile::PointVec{Point2f(0, 1), Point2f(1, 0)});
}

std::string Taper::ToString() const {
    return "TAP [ " + Util::EnumName(axis) + " / " + profile.ToString();
}
