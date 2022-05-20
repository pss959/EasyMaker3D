#pragma once

#include <vector>

#include "App/SelPath.h"
#include "Executors/Executor.h"

/// ChangeMirrorExecutor executes the ChangeMirrorCommand.
///
/// \ingroup Executors
class ChangeMirrorExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeMirrorCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeMirrorCommand.
    struct ExecData_ : public Command::ExecData {
        std::vector<SelPath> paths_to_models;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
