#include "Commands/CreateImportedModelCommand.h"

#include "Util/Enum.h"

void CreateImportedModelCommand::AddFields() {
    AddField(path_.Init("path"));

    CreateModelCommand::AddFields();
}

void CreateImportedModelCommand::CreationDone() {
    CreateModelCommand::CreationDone();
    if (path_.WasSet())
        path_ = FixPath(GetPath());
}

std::string CreateImportedModelCommand::GetDescription() const {
    return "Create Imported Model " + GetResultName();
}
