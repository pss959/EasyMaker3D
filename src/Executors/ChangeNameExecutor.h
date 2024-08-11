//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

/// ChangeNameExecutor executes the ChangeNameCommand.
///
/// \ingroup Executors
class ChangeNameExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeNameCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
