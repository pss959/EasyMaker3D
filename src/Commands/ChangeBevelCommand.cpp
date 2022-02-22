#include "Commands/ChangeBevelCommand.h"

#include "Parser/Registry.h"

void ChangeBevelCommand::AddFields() {
    AddField(profile_points_);
    AddField(bevel_scale_);
    AddField(max_angle_);
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
    if (max_deg < 0 || max_deg > Defaults::kMaxBevelAngle) {
        details = "Maximum angle is out of range";
        return false;
    }
    return true;
}

std::string ChangeBevelCommand::GetDescription() const {
    return "Changed the bevel of " + GetModelsDesc(GetModelNames());
}

Bevel ChangeBevelCommand::GetBevel() const {
    Bevel bevel;
    bevel.profile.SetPoints(profile_points_);
    bevel.scale = bevel_scale_;
    bevel.max_angle = max_angle_;
    return bevel;
}
