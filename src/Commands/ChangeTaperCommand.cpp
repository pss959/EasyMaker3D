#include "Commands/ChangeTaperCommand.h"

#include "Models/TaperedModel.h"
#include "Parser/Registry.h"
#include "Util/Tuning.h"

void ChangeTaperCommand::AddFields() {
    AddField(profile_points_.Init("profile_points"));
    AddField(taper_scale_.Init("taper_scale", 1));
    AddField(max_angle_.Init("max_angle",
                             Anglef::FromDegrees(TK::kDefaultMaxBevelAngle)));

    MultiModelCommand::AddFields();
}

bool ChangeTaperCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (taper_scale_ <= 0) {
        details = "Taper scale is not positive";
        return false;
    }
    const float max_deg = max_angle_.GetValue().Degrees();
    if (max_deg < 0 || max_deg > 180) {
        details = "Maximum angle is out of range";
        return false;
    }
    return true;
}

std::string ChangeTaperCommand::GetDescription() const {
    return "Changed the taper of " + GetModelsDesc(GetModelNames());
}

void ChangeTaperCommand::SetTaper(const Bevel &taper) {
    profile_points_ = taper.profile.GetMovablePoints();
    taper_scale_    = taper.scale;
    max_angle_      = taper.max_angle;
}

Bevel ChangeTaperCommand::GetTaper() const {
    Bevel taper;
    taper.profile   = TaperedModel::CreateProfile(profile_points_);
    taper.scale     = taper_scale_;
    taper.max_angle = max_angle_;
    return taper;
}
