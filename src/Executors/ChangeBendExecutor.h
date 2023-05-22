#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Bend.h"
#include "Selection/SelPath.h"

/// ChangeBendExecutor executes the ChangeBendCommand.
///
/// \ingroup Executors
class ChangeBendExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeBendCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeBendCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            Bend     old_bend;
            Vector3f old_translation;
            Vector3f new_translation;
            Vector3f base_translation;  /// Without any offset.
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
