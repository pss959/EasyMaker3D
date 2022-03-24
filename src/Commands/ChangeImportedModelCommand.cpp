#include "Commands/ChangeImportedModelCommand.h"

void ChangeImportedModelCommand::AddFields() {
    AddField(new_path_);
    SingleModelCommand::AddFields();
}

std::string ChangeImportedModelCommand::GetDescription() const {
    return "Changed the import path in " + GetModelDesc(GetModelName());
}
