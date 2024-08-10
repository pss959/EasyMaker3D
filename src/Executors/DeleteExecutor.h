#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

class DeleteCommand;

/// DeleteExecutor executes the DeleteCommand.
///
/// \ingroup Executors
class DeleteExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "DeleteCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a DeleteCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath path_to_model;
            int     index = -1;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
