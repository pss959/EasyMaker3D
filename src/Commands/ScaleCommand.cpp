#include "Commands/ScaleCommand.h"

void ScaleCommand::AddFields() {
    AddField(ratios_);
    AddField(is_symmetric_);
    MultiModelCommand::AddFields();
}

bool ScaleCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    const Vector3f ratios = GetRatios();
    if (ratios[0] == 0 || ratios[1] == 0 || ratios[2] == 0) {
        details = "Invalid scale by zero";
        return false;
    }
    if (IsSymmetric() && (ratios[0] <= 0 || ratios[1] < 0 || ratios[2] <= 0)) {
        details = "Invalid negative asymmetric scale";
        return false;
    }
    return true;
}

std::string ScaleCommand::GetDescription() const {
    return "Scaled " + GetModelsDesc(GetModelNames()) +
        (IsSymmetric() ? " symmetrically " : " asymmetrically ");
}
