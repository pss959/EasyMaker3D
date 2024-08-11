//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeNameCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"

void ChangeNameCommand::AddFields() {
    AddField(new_name_.Init("new_name"));

    SingleModelCommand::AddFields();
}

bool ChangeNameCommand::IsValid(Str &details) {
    if (! SingleModelCommand::IsValid(details))
        return false;
    if (! Model::IsValidName(new_name_)) {
        details = "Invalid new model name";
        return false;
    }
    return true;
}

Str ChangeNameCommand::GetDescription() const {
    return "Changed the name of " + GetModelDesc(GetModelName());
}

void ChangeNameCommand::SetNewName(const Str &new_name) {
    ASSERT(Model::IsValidName(new_name));
    new_name_ = new_name;
}
