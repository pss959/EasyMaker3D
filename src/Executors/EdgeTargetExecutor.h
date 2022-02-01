#pragma once

#include "Executors/Executor.h"

/// EdgeTargetExecutor executes the ChangeEdgeTargetCommand.
/// \ingroup Executors
class EdgeTargetExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeEdgeTargetCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
