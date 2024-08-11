//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/MultiModelCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"
#include "Util/General.h"

void MultiModelCommand::AddFields() {
    AddField(model_names_.Init("model_names"));

    Command::AddFields();
}

bool MultiModelCommand::IsValid(Str &details) {
    if (! Command::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (GetModelNames().empty()) {
        details = "Missing model names";
        return false;
    }
    for (const auto &name: GetModelNames()) {
        if (! Model::IsValidName(name)) {
            details = "Invalid model name: '" + name + "'";
            return false;
        }
    }
    return true;
}

void MultiModelCommand::SetFromSelection(const Selection &sel) {
    ASSERT(sel.HasAny());
    SetModelNames(
        Util::ConvertVector<Str, SelPath>(
            sel.GetPaths(),
            [](const SelPath &path){ return path.GetModel()->GetName(); }));
}
