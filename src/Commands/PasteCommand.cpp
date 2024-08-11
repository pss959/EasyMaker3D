//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/PasteCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"
#include "Util/General.h"

void PasteCommand::AddFields() {
    AddField(parent_name_.Init("parent_name"));

    Command::AddFields();
}

Str PasteCommand::GetDescription() const {
    Str desc = "Pasted " + GetModelsDesc(model_names_);
    const auto &parent = parent_name_.GetValue();
    if (! parent.empty())
        desc += " into Model \"" + parent + "\"";
    return desc;
}

void PasteCommand::SetModelsForDescription(
    const std::vector<ModelPtr> &models) {
    model_names_ = Util::ConvertVector<Str, ModelPtr>(
        models, [](const ModelPtr &m){ return m->GetName(); });
}
