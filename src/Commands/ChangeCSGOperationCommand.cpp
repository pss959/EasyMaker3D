//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeCSGOperationCommand.h"

#include "Models/Model.h"

void ChangeCSGOperationCommand::AddFields() {
    AddField(new_operation_.Init("new_operation", CSGOperation::kUnion));
    AddField(result_names_.Init("result_names"));

    MultiModelCommand::AddFields();
}

bool ChangeCSGOperationCommand::IsValid(Str &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (GetResultNames().empty()) {
        details = "Missing result names";
        return false;
    }
    for (const auto &name: GetResultNames()) {
        if (! Model::IsValidName(name)) {
            details = "Invalid result model name: '" + name + "'";
            return false;
        }
    }
    return true;
}

Str ChangeCSGOperationCommand::GetDescription() const {
    return "Changed the CSG operation of " + GetModelsDesc(GetModelNames()) +
        " to " + new_operation_.GetEnumWords();
}
