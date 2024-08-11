//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CreateExtrudedModelCommand.h"

#include "Util/Enum.h"

Str CreateExtrudedModelCommand::GetDescription() const {
    return BuildDescription("extruded");
}
