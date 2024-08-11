//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeColorCommand.h"

void ChangeColorCommand::AddFields() {
    AddField(new_color_.Init("new_color"));

    MultiModelCommand::AddFields();
}

Str ChangeColorCommand::GetDescription() const {
    return "Changed the color of " + GetModelsDesc(GetModelNames());
}
