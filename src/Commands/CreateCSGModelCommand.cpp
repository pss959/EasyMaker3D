#include "Commands/CreateCSGModelCommand.h"

void CreateCSGModelCommand::AddFields() {
    AddField(operation_);
    MultiModelCommand::AddFields();
}

bool CreateCSGModelCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (GetModelNames().size() < 2U) {
        details = "Need at least two model names";
        return false;
    }
    return true;
}

std::string CreateCSGModelCommand::GetDescription() const {
    return "Create a CSG " + operation_.GetEnumName() + " from " +
        GetModelsDesc(GetModelNames());
}
