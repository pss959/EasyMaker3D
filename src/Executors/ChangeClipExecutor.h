#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Types.h"
#include "Selection/SelPath.h"

class ClippedModel;

/// ChangeClipExecutor executes the ChangeClipCommand.
///
/// \ingroup Executors
class ChangeClipExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeClipCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeClipCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath path_to_model;
            Plane   old_plane;  ///< In object coordinates.
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
