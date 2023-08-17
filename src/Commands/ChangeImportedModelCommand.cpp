#include "Commands/ChangeImportedModelCommand.h"

void ChangeImportedModelCommand::AddFields() {
    AddField(new_path_.Init("new_path"));

    SingleModelCommand::AddFields();
}

bool ChangeImportedModelCommand::IsValid(Str &details) {
    if (! SingleModelCommand::IsValid(details))
        return false;
    if (GetNewPath().empty()) {
        details = "Missing new path to model";
        return false;
    }
    return true;
}

void ChangeImportedModelCommand::CreationDone() {
    SingleModelCommand::CreationDone();
    if (new_path_.WasSet())
        new_path_ = FixPath(GetNewPath());
}

Str ChangeImportedModelCommand::GetDescription() const {
    return "Changed the import path in " + GetModelDesc(GetModelName());
}
