//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ConvertCommand.h"

#include "Models/Model.h"

void ConvertCommand::AddFields() {
    AddField(result_names_.Init("result_names"));

    MultiModelCommand::AddFields();
}

bool ConvertCommand::IsValid(Str &details) {
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

Str ConvertCommand::BuildDescription(const Str &op) const {
    return "Applied " + op + " to " + GetModelsDesc(GetModelNames());
}
