#pragma once

#include <vector>

#include "Enums/CSGOperation.h"
#include "Executors/Executor.h"
#include "Selection/SelPath.h"

/// ChangeCSGExecutor executes the ChangeCSGOperationCommand.
///
/// \ingroup Executors
class ChangeCSGExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeCSGOperationCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeCSGOperationCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath      path_to_model;
            CSGOperation old_operation;
            std::string  old_name;
            std::string  new_name;
            Vector3f     old_translation;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
