//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/ChangeSpinExecutor.h"

/// ChangeTwistExecutor executes the ChangeTwistCommand.
///
/// \ingroup Executors
class ChangeTwistExecutor : public ChangeSpinExecutor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeTwistCommand";
    }

  protected:
    // Required ChangeSpinExecutor functions.
    virtual Spin GetModelSpin(const Model &model) const override;
    virtual void SetModelSpin(Model &model, const Spin &spin) const override;
};
