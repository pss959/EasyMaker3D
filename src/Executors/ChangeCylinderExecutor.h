#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

/// ChangeCylinderExecutor executes the ChangeCylinderCommand.
///
/// \ingroup Executors
class ChangeCylinderExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeCylinderCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeCylinderCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            float    old_radius;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
