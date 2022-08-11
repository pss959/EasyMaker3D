#include "Commands/ScaleCommand.h"

void ScaleCommand::AddFields() {
    AddField(ratios_.Init("ratios", Vector3f(1, 1, 1)));
    AddField(mode_.Init("mode",     Mode::kAsymmetric));

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
    if (GetMode() != Mode::kAsymmetric &&
        (ratios[0] <= 0 || ratios[1] < 0 || ratios[2] <= 0)) {
        details = "Invalid negative asymmetric scale";
        return false;
    }
    return true;
}

std::string ScaleCommand::GetDescription() const {
    const Mode mode = GetMode();
    return "Scaled " + GetModelsDesc(GetModelNames()) +
        (mode == Mode::kAsymmetric ? " asymmetrically " :
         mode == Mode::kCenterSymmetric ? " symmetrically about the center " :
         " symmetrically about the base center ");
}
