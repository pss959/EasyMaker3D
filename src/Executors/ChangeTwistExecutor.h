#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Types.h"
#include "Models/TwistedModel.h"
#include "Selection/SelPath.h"

/// ChangeTwistExecutor executes the ChangeTwistCommand.
///
/// \ingroup Executors
class ChangeTwistExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeTwistCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeTwistCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath             path_to_model;
            TwistedModel::Twist old_twist;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
