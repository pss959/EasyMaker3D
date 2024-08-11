//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CombineHullModelCommand.h"

Str CombineHullModelCommand::GetDescription() const {
    return BuildDescription("convex hull");
}
