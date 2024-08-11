//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CreatePrimitiveModelCommand.h"

#include "Util/Enum.h"

void CreatePrimitiveModelCommand::AddFields() {
    AddField(type_.Init("type", PrimitiveType::kBox));

    CreateModelCommand::AddFields();
}

Str CreatePrimitiveModelCommand::GetDescription() const {
    return BuildDescription(type_.GetEnumWords());
}
