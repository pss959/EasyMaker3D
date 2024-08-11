//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/Executor.h"

/// PointTargetExecutor executes the ChangePointTargetCommand.
///
/// \ingroup Executors
class PointTargetExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangePointTargetCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
