#include "Commands/ChangeRevSurfCommand.h"

#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeRevSurfCommand::AddFields() {
    AddField(profile_points_.Init("profile_points"));
    AddField(sweep_angle_.Init("sweep_angle", Anglef::FromDegrees(360)));

    MultiModelCommand::AddFields();
}

bool ChangeRevSurfCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (profile_points_.GetValue().size() == 0) {
        details = "Not enough profile points";
        return false;
    }
    const float sweep_deg = sweep_angle_.GetValue().Degrees();
    if (sweep_deg <= 0 || sweep_deg > 360) {
        details = "Sweep angle is out of range";
        return false;
    }
    return true;
}

std::string ChangeRevSurfCommand::GetDescription() const {
    return "Changed the profile or sweep angle in " +
        GetModelsDesc(GetModelNames());
}

void ChangeRevSurfCommand::SetProfile(const Profile &profile) {
    // Make sure the profile is set up for a RevSurfModel.
    ASSERT(profile.GetStartPoint() == Point2f(0, 1));
    ASSERT(profile.GetEndPoint()   == Point2f::Zero());

    profile_points_ = profile.GetPoints();
}

Profile ChangeRevSurfCommand::GetProfile() const {
    Profile profile(Point2f(0, 1), Point2f::Zero());
    profile.SetPoints(profile_points_);
    return profile;
}

void ChangeRevSurfCommand::SetSweepAngle(const Anglef &sweep_angle) {
    ASSERT(sweep_angle.Degrees() > 0 && sweep_angle.Degrees() <= 360);
    sweep_angle_ = sweep_angle;
}
