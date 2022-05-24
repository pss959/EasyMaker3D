#include "Commands/ChangeCSGOperationCommand.h"

#include "Models/Model.h"

void ChangeCSGOperationCommand::AddFields() {
    AddField(new_operation_);
    AddField(result_names_);
    MultiModelCommand::AddFields();
}

bool ChangeCSGOperationCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;

    if (GetResultNames().empty()) {
        details = "missing result names";
        return false;
    }
    for (const auto &name: GetResultNames()) {
        if (! Model::IsValidName(name)) {
            details = "invalid result model name: '" + name + "'";
            return false;
        }
    }
    return true;
}

std::string ChangeCSGOperationCommand::GetDescription() const {
    return "Change the CSG operation of " + GetModelsDesc(GetModelNames()) +
        " to " + new_operation_.GetEnumWords();
}

// XXXX Implement and use SetResultNames() !!!!
