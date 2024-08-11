//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeClipCommand.h"

Str ChangeClipCommand::GetDescription() const {
    return "Changed the clip plane of " + GetModelsDesc(GetModelNames()) +
        " to " + GetPlane().ToString();
}
