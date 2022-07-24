#include "Commands/CreateImportedModelCommand.h"

#include "Util/Enum.h"

void CreateImportedModelCommand::AddFields() {
    AddField(path_.Init("path"));

    CreateModelCommand::AddFields();
}

std::string CreateImportedModelCommand::GetDescription() const {
    return "Create Imported Model " + GetResultName();
}
