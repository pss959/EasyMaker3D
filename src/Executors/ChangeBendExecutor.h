//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/ChangeSpinExecutor.h"

/// ChangeBendExecutor executes the ChangeBendCommand.
///
/// \ingroup Executors
class ChangeBendExecutor : public ChangeSpinExecutor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeBendCommand";
    }

  protected:
    // Required ChangeSpinExecutor functions.
    virtual Spin GetModelSpin(const Model &model) const override;
    virtual void SetModelSpin(Model &model, const Spin &spin) const override;
};
