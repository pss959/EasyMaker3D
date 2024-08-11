//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeComplexityCommand.h"

void ChangeComplexityCommand::AddFields() {
    AddField(new_complexity_.Init("new_complexity", 0));

    MultiModelCommand::AddFields();
}

Str ChangeComplexityCommand::GetDescription() const {
    return "Changed the complexity of " + GetModelsDesc(GetModelNames());
}
