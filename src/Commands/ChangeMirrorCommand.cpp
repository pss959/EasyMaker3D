//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeMirrorCommand.h"

Str ChangeMirrorCommand::GetDescription() const {
    return "Mirrored " + GetModelsDesc(GetModelNames()) + " across plane " +
        GetPlane().ToString();
}
