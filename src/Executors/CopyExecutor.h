#pragma once

#include "Executors/Executor.h"

/// CopyExecutor executes the CopyCommand.
///
/// \ingroup Executors
class CopyExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CopyCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
