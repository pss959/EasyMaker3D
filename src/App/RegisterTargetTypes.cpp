//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Parser/Registry.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterTargetTypes_();

void RegisterTargetTypes_() {
    REGISTER_TYPE_(EdgeTarget);
    REGISTER_TYPE_(PointTarget);
}

#undef REGISTER_TYPE_

