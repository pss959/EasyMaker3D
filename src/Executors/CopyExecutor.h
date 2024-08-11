//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Executors/Executor.h"

/// CopyExecutor executes the CopyCommand.
///
/// \ingroup Executors
class CopyExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CopyCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
