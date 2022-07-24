#include "Commands/ChangeCylinderCommand.h"

#include "Base/Tuning.h"
#include "Models/CylinderModel.h"
#include "Parser/Registry.h"

void ChangeCylinderCommand::AddFields() {
    AddField(is_top_radius_.Init("is_top_radius", true));
    AddField(new_radius_.Init("new_radius"));

    MultiModelCommand::AddFields();
}

bool ChangeCylinderCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (new_radius_ <= TK::kMinCylinderRadius) {
        details = "Radius is smaller than minimum";
        return false;
    }
    return true;
}

std::string ChangeCylinderCommand::GetDescription() const {
    return std::string("Changed the ") +
        (is_top_radius_.GetValue() ? "top" : "bottom") + " radius of " +
        GetModelsDesc(GetModelNames());
}
