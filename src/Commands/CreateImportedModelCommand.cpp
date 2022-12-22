#include "Commands/CreateImportedModelCommand.h"

std::string CreateImportedModelCommand::GetDescription() const {
    return "Create Imported Model " + GetResultName();
}
