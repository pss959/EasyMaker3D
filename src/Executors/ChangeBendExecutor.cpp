//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeBendExecutor.h"

#include "Models/BentModel.h"

Spin ChangeBendExecutor::GetModelSpin(const Model &model) const {
    ASSERT(dynamic_cast<const BentModel *>(&model));
    return static_cast<const BentModel &>(model).GetSpin();
}

void ChangeBendExecutor::SetModelSpin(Model &model, const Spin &spin) const {
    ASSERT(dynamic_cast<BentModel *>(&model));
    static_cast<BentModel &>(model).SetSpin(spin);
}
