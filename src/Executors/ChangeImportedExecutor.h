#pragma once

#include <string>

#include "Executors/Executor.h"
#include "Models/ImportedModel.h"

/// ChangeImportedExecutor executes the ChangeImportedModelCommand.
///
/// \ingroup Executors
class ChangeImportedExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeImportedModelCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo ChangeImportedModelCommand.
    struct ExecData_ : public Command::ExecData {
        ImportedModelPtr imported_model;
        std::string      old_path;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
