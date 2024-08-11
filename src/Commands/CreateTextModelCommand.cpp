//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CreateTextModelCommand.h"

#include "Util/Enum.h"

void CreateTextModelCommand::AddFields() {
    AddField(text_.Init("text", "A"));

    CreateModelCommand::AddFields();
}

Str CreateTextModelCommand::GetDescription() const {
    return BuildDescription("3D text");
}
