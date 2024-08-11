//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ConvertBendExecutor.h"

#include "Models/BentModel.h"

ConvertedModelPtr ConvertBendExecutor::CreateConvertedModel(const Str &name) {
    return Model::CreateModel<BentModel>(
        name.empty() ? CreateUniqueName("Bent") : name);
}
