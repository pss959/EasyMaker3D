#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Taper.h"
#include "Selection/SelPath.h"

/// ChangeTaperExecutor executes the ChangeTaperCommand.
///
/// \ingroup Executors
class ChangeTaperExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeTaperCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeTaperCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath path_to_model;
            Taper   old_taper;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
