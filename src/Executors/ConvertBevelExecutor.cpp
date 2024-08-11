//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ConvertBevelExecutor.h"

#include "Models/BeveledModel.h"

ConvertedModelPtr ConvertBevelExecutor::CreateConvertedModel(const Str &name) {
    return Model::CreateModel<BeveledModel>(
        name.empty() ? CreateUniqueName("Beveled") : name);
}
