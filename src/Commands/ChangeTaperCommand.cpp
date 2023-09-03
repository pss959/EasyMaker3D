#include "Commands/ChangeTaperCommand.h"

#include "Models/TaperedModel.h"
#include "Parser/Registry.h"

void ChangeTaperCommand::AddFields() {
    AddField(axis_.Init("axis", Taper().axis));
    AddField(profile_points_.Init("profile_points"));
    MultiModelCommand::AddFields();
}

bool ChangeTaperCommand::IsValid(Str &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! Taper::IsValidProfile(GetTaper().profile)) {
        details = "Invalid profile";
        return false;
    }
    return true;
}

Str ChangeTaperCommand::GetDescription() const {
    return "Changed the taper of " + GetModelsDesc(GetModelNames());
}

void ChangeTaperCommand::SetTaper(const Taper &taper) {
    ASSERT(Taper::IsValidProfile(taper.profile));
    axis_           = taper.axis;
    profile_points_ = taper.profile.GetPoints();
}

Taper ChangeTaperCommand::GetTaper() const {
    Taper taper;
    taper.axis    = axis_;
    taper.profile = TaperedModel::CreateProfile(profile_points_);
    return taper;
}
