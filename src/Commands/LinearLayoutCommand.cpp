//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/LinearLayoutCommand.h"

#include "Place/EdgeTarget.h"

void LinearLayoutCommand::AddFields() {
    AddField(offset_.Init("offset", Vector3f::Zero()));

    MultiModelCommand::AddFields();
}

Str LinearLayoutCommand::GetDescription() const {
    return "Linear layout of " + GetModelsDesc(GetModelNames());
}

void LinearLayoutCommand::SetFromTarget(const EdgeTarget &target) {
    offset_ = target.GetLength() * target.GetDirection();
}
