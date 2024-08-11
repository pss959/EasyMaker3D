//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeTwistExecutor.h"

#include "Models/TwistedModel.h"

Spin ChangeTwistExecutor::GetModelSpin(const Model &model) const {
    ASSERT(dynamic_cast<const TwistedModel *>(&model));
    return static_cast<const TwistedModel &>(model).GetSpin();
}

void ChangeTwistExecutor::SetModelSpin(Model &model, const Spin &spin) const {
    ASSERT(dynamic_cast<TwistedModel *>(&model));
    static_cast<TwistedModel &>(model).SetSpin(spin);
}
