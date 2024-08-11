//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ConvertTwistExecutor.h"

#include "Models/TwistedModel.h"

ConvertedModelPtr ConvertTwistExecutor::CreateConvertedModel(const Str &name) {
    return Model::CreateModel<TwistedModel>(
        name.empty() ? CreateUniqueName("Twisted") : name);
}
