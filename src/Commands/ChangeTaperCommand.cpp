#include "Commands/ChangeTaperCommand.h"

#include "Models/TaperedModel.h"
#include "Parser/Registry.h"

void ChangeTaperCommand::AddFields() {
    AddField(axis_.Init("axis"));
    AddField(profile_points_.Init("profile_points"));

    MultiModelCommand::AddFields();
}

bool ChangeTaperCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! GetTaper().profile.IsValid()) {
        details = "Profile is not valid";
        return false;
    }
    return true;
}

std::string ChangeTaperCommand::GetDescription() const {
    return "Changed the taper of " + GetModelsDesc(GetModelNames());
}

void ChangeTaperCommand::SetTaper(const Taper &taper) {
    axis_           = taper.axis;
    profile_points_ = taper.profile.GetMovablePoints();
}

Taper ChangeTaperCommand::GetTaper() const {
    Taper taper;
    taper.axis    = axis_;
    taper.profile = TaperedModel::CreateProfile(profile_points_);
    return taper;
}
