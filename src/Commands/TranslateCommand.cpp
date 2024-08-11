//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/TranslateCommand.h"

void TranslateCommand::AddFields() {
    AddField(translation_.Init("translation", Vector3f::Zero()));

    MultiModelCommand::AddFields();
}

Str TranslateCommand::GetDescription() const {
    return "Translated " + GetModelsDesc(GetModelNames());
}
