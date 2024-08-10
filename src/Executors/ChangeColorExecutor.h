#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Color.h"
#include "Math/Types.h"
#include "Selection/SelPath.h"

/// ChangeColorExecutor executes the ChangeColorCommand.
///
/// \ingroup Executors
class ChangeColorExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeColorCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeColorCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath path_to_model;
            Color   old_color;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
