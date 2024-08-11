//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/RotateCommand.h"

void RotateCommand::AddFields() {
    AddField(rotation_.Init("rotation"));
    AddField(is_in_place_.Init("is_in_place", false));
    AddField(is_axis_aligned_.Init("is_axis_aligned", false));

    MultiModelCommand::AddFields();
}

Str RotateCommand::GetDescription() const {
    return "Rotated " + GetModelsDesc(GetModelNames()) +
        (IsInPlace() ? " (in place)" : "");
}
