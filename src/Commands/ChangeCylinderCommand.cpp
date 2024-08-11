//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeCylinderCommand.h"

#include "Models/CylinderModel.h"
#include "Parser/Registry.h"

void ChangeCylinderCommand::AddFields() {
    AddField(is_top_radius_.Init("is_top_radius", true));
    AddField(new_radius_.Init("new_radius", 0));

    MultiModelCommand::AddFields();
}

Str ChangeCylinderCommand::GetDescription() const {
    return Str("Changed the ") +
        (is_top_radius_.GetValue() ? "top" : "bottom") + " radius of " +
        GetModelsDesc(GetModelNames());
}
