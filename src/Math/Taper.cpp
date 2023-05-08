#include "Math/Taper.h"

#include "Util/Enum.h"
#include "Util/String.h"

Profile Taper::CreateDefaultProfile() {
    return Profile(Profile::Type::kOpen, 2,
                   Profile::PointVec{Point2f(0, 1), Point2f(1, 0)});
}

bool Taper::IsValidProfile(const Profile &prof) {
    // Must be a valid open Profile.
    if (prof.GetType() != Profile::Type::kOpen || ! prof.IsValid())
        return false;

    // There must be at least 2 profile points.
    const auto &pts = prof.GetPoints();
    if (pts.size() < 2U)
        return false;

    // The end points must be at Y=1 and Y=0.
    if (pts.front()[1] != 1 || pts.back()[1] != 0)
        return false;

    // All other points must be monotonically decreasing in Y.
    for (size_t i = 1; i + 1 < pts.size(); ++i)
        if (pts[i - 1][1] <= pts[i][1])
            return false;

    return true;
}


std::string Taper::ToString() const {
    return "TAP [ " + Util::EnumName(axis) + " / " + profile.ToString();
}
