//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeOrderCommand.h"

void ChangeOrderCommand::AddFields() {
    AddField(is_previous_.Init("is_previous", false));

    SingleModelCommand::AddFields();
}

Str ChangeOrderCommand::GetDescription() const {
    return "Moved " + GetModelDesc(GetModelName()) +
        (IsPrevious() ? " earlier" : " later") + " in the order";
}
