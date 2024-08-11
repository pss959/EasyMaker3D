//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/DeleteCommand.h"

Str DeleteCommand::GetDescription() const {
    return "Deleted " + GetModelsDesc(GetModelNames());
}
