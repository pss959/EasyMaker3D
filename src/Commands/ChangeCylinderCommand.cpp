#include "Commands/ChangeCylinderCommand.h"

#include "Models/CylinderModel.h"
#include "Parser/Registry.h"

void ChangeCylinderCommand::AddFields() {
    AddField(is_top_radius_);
    AddField(new_radius_);
    MultiModelCommand::AddFields();
}

bool ChangeCylinderCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (new_radius_ <= CylinderModel::kMinRadius) {
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
