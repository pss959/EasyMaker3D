#pragma once

#include <vector>

#include "App/SelPath.h"
#include "Executors/Executor.h"

/// ChangeNameExecutor executes the ChangeNameCommand.
///
/// \ingroup Executors
class ChangeNameExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeNameCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;
};
