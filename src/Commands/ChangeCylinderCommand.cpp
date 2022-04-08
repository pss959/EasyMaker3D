#include "Commands/ChangeCylinderCommand.h"

#include "Parser/Registry.h"

void ChangeCylinderCommand::AddFields() {
    AddField(which_radius_);
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
    return "Changed the " + which_radius_.GetEnumWords() + " radius of " +
        GetModelsDesc(GetModelNames());
}
