//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ConvertTaperExecutor.h"

#include "Models/TaperedModel.h"

ConvertedModelPtr ConvertTaperExecutor::CreateConvertedModel(
    const Str &name) {
    return Model::CreateModel<TaperedModel>(
        name.empty() ? CreateUniqueName("Tapered") : name);
}
