#include "Commands/CreateCSGModelCommand.h"

void CreateCSGModelCommand::AddFields() {
    AddField(operation_.Init("operation", CSGOperation::kUnion));

    CombineCommand::AddFields();
}

bool CreateCSGModelCommand::IsValid(std::string &details) {
    if (! CombineCommand::IsValid(details))
        return false;
    if (GetModelNames().size() < 2U) {
        details = "Need at least two model names";
        return false;
    }
    return true;
}

std::string CreateCSGModelCommand::GetDescription() const {
    return "Create CSG Model " + GetResultName() + " from " +
        GetModelsDesc(GetModelNames());
}
