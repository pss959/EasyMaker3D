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
    if (! Taper::IsValidProfile(GetTaper().profile)) {
        details = "Invalid profile";
        return false;
    }
    return true;
}

std::string ChangeTaperCommand::GetDescription() const {
    return "Changed the taper of " + GetModelsDesc(GetModelNames());
}

void ChangeTaperCommand::SetTaper(const Taper &taper) {
    if (! Taper::IsValidProfile(taper.profile))
        std::cerr << "XXXX BAD TAPER: " << taper.ToString() << "\n";

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
