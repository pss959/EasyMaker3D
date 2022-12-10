#include "Commands/ChangeBevelCommand.h"

#include "Base/Tuning.h"
#include "Parser/Registry.h"

void ChangeBevelCommand::AddFields() {
    AddField(profile_points_.Init("profile_points"));
    AddField(bevel_scale_.Init("bevel_scale", 1));
    AddField(max_angle_.Init("max_angle",
                             Anglef::FromDegrees(TK::kDefaultMaxBevelAngle)));

    MultiModelCommand::AddFields();
}

bool ChangeBevelCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (bevel_scale_ <= 0) {
        details = "Bevel scale is not positive";
        return false;
    }
    const float max_deg = max_angle_.GetValue().Degrees();
    if (max_deg < 0 || max_deg > 180) {
        details = "Maximum angle is out of range";
        return false;
    }
    return true;
}

std::string ChangeBevelCommand::GetDescription() const {
    return "Changed the bevel of " + GetModelsDesc(GetModelNames());
}

void ChangeBevelCommand::SetBevel(const Bevel &bevel) {
    profile_points_ = bevel.profile.GetPoints();
    bevel_scale_    = bevel.scale;
    max_angle_      = bevel.max_angle;
}

Bevel ChangeBevelCommand::GetBevel() const {
    Bevel bevel;
    bevel.profile.SetPoints(profile_points_);
    bevel.scale     = bevel_scale_;
    bevel.max_angle = max_angle_;
    return bevel;
}
