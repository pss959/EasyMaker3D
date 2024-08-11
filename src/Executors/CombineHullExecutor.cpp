//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/CombineHullExecutor.h"

#include "Commands/CombineHullModelCommand.h"
#include "Models/HullModel.h"

CombinedModelPtr CombineHullExecutor::CreateCombinedModel(
    Command &command, const Str &name) {
    return Model::CreateModel<HullModel>(name.empty() ?
                                         CreateUniqueName("Hull") : name);
}
