#include "Commands/ChangeTorusCommand.h"

#include "Base/Tuning.h"
#include "Models/TorusModel.h"
#include "Parser/Registry.h"

void ChangeTorusCommand::AddFields() {
    AddField(is_inner_radius_.Init("is_inner_radius", true));
    AddField(new_radius_.Init("new_radius"));

    MultiModelCommand::AddFields();
}

bool ChangeTorusCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (is_inner_radius_) {
        if (new_radius_ < TK::kMinTorusInnerRadius) {
            details = "Inner radius is smaller than minimum";
            return false;
        }
    }
    else {
        if (new_radius_ < TorusModel::GetMinOuterRadiusForInnerRadius(
                TK::kMinTorusInnerRadius)) {
            details = "Outer radius is smaller than minimum";
            return false;
        }
    }
    return true;
}

std::string ChangeTorusCommand::GetDescription() const {
    return std::string("Changed the ") +
        (is_inner_radius_.GetValue() ? "inner" : "outer") + " radius of " +
        GetModelsDesc(GetModelNames());
}
