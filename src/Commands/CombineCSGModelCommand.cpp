#include "Commands/CombineCSGModelCommand.h"

void CombineCSGModelCommand::AddFields() {
    AddField(operation_.Init("operation", CSGOperation::kUnion));

    CombineCommand::AddFields();
}

bool CombineCSGModelCommand::IsValid(Str &details) {
    if (! CombineCommand::IsValid(details))
        return false;
    if (GetModelNames().size() < 2U) {
        details = "Need at least two model names";
        return false;
    }
    return true;
}

Str CombineCSGModelCommand::GetDescription() const {
    return BuildDescription("CSG");
}
