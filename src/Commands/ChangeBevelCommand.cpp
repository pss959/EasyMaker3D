#include "Commands/ChangeBevelCommand.h"

#include "Models/BeveledModel.h"
#include "Parser/Registry.h"
#include "Util/Tuning.h"

void ChangeBevelCommand::AddFields() {
    AddField(profile_points_.Init("profile_points"));
    AddField(bevel_scale_.Init("bevel_scale", 1));
    AddField(max_angle_.Init("max_angle",
                             Anglef::FromDegrees(TK::kDefaultMaxBevelAngle)));

    MultiModelCommand::AddFields();
}

bool ChangeBevelCommand::IsValid(Str &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    const Bevel bevel = GetBevel();
    if (! bevel.profile.IsValid()) {
        details = "Invalid profile";
        return false;
    }
    if (bevel.scale <= 0) {
        details = "Bevel scale is not positive";
        return false;
    }
    const float max_deg = bevel.max_angle.Degrees();
    if (max_deg < 0 || max_deg > 180) {
        details = "Maximum angle is out of range";
        return false;
    }
    return true;
}

Str ChangeBevelCommand::GetDescription() const {
    return "Changed the bevel of " + GetModelsDesc(GetModelNames());
}

void ChangeBevelCommand::SetBevel(const Bevel &bevel) {
    ASSERT(bevel.profile.IsValid());
    profile_points_ = bevel.profile.GetMovablePoints();
    bevel_scale_    = bevel.scale;
    max_angle_      = bevel.max_angle;
}

Bevel ChangeBevelCommand::GetBevel() const {
    Bevel bevel;
    bevel.profile   = BeveledModel::CreateProfile(profile_points_);
    bevel.scale     = bevel_scale_;
    bevel.max_angle = max_angle_;
    return bevel;
}
