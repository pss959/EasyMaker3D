#include "Commands/CreateImportedModelCommand.h"

#include "Util/Enum.h"

void CreateImportedModelCommand::AddFields() {
    AddField(path_);
    CreateModelCommand::AddFields();
}

std::string CreateImportedModelCommand::GetDescription() const {
    return "Create an Imported Model";
}
