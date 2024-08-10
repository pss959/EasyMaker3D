#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

/// ChangeOrderExecutor executes the ChangeOrderCommand.
///
/// \ingroup Executors
class ChangeOrderExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeOrderCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
