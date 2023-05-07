#include "Commands/ChangeExtrudedCommand.h"

#include "Models/ExtrudedModel.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeExtrudedCommand::AddFields() {
    AddField(profile_points_.Init("profile_points"));

    MultiModelCommand::AddFields();
}

bool ChangeExtrudedCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (profile_points_.GetValue().size() < 3U) {
        details = "Not enough profile points";
        return false;
    }
    return true;
}

std::string ChangeExtrudedCommand::GetDescription() const {
    return "Changed the profile in " + GetModelsDesc(GetModelNames());
}

void ChangeExtrudedCommand::SetProfile(const Profile &profile) {
    // Make sure the profile is set up for a ExtrudedModel.
    ASSERT(profile.GetType() == Profile::Type::kClosed);
    ASSERT(profile.GetMinPointCount() == 3U);

    profile_points_ = profile.GetMovablePoints();
}

Profile ChangeExtrudedCommand::GetProfile() const {
    return ExtrudedModel::CreateProfile(profile_points_);
}
