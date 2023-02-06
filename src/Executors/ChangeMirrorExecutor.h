#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Types.h"
#include "Selection/SelPath.h"

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
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            Vector3f object_plane_normal;
            Vector3f old_translation;
            Vector3f new_translation;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
