#pragma once

#include <vector>

#include "App/SelPath.h"
#include "Executors/Executor.h"

/// ChangeOrderExecutor executes the ChangeOrderCommand.
///
/// \ingroup Executors
class ChangeOrderExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeOrderCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
