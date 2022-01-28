#pragma once

#include "Executors/Executor.h"

/// PointTargetExecutor executes the ChangePointTargetCommand.
/// \ingroup Executors
class PointTargetExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangePointTargetCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
