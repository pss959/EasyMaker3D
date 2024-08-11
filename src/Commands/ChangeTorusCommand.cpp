//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeTorusCommand.h"

#include "Models/TorusModel.h"
#include "Parser/Registry.h"
#include "Util/Tuning.h"

void ChangeTorusCommand::AddFields() {
    AddField(is_inner_radius_.Init("is_inner_radius", true));
    AddField(new_radius_.Init("new_radius"));

    MultiModelCommand::AddFields();
}

bool ChangeTorusCommand::IsValid(Str &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (is_inner_radius_ && new_radius_ < TK::kMinTorusInnerRadius) {
        details = "Inner radius is smaller than minimum";
        return false;
    }
    if (! is_inner_radius_ &&
        new_radius_ <
        TorusModel::GetMinOuterRadiusForInnerRadius(TK::kMinTorusInnerRadius)) {
        details = "Outer radius is smaller than minimum";
        return false;
    }
    return true;
}

Str ChangeTorusCommand::GetDescription() const {
    return Str("Changed the ") +
        (is_inner_radius_.GetValue() ? "inner" : "outer") + " radius of " +
        GetModelsDesc(GetModelNames());
}
