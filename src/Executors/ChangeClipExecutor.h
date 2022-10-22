#pragma once

#include <vector>

#include "App/SelPath.h"
#include "Executors/Executor.h"

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
            SelPath  path_to_model;
            Plane    local_plane;
            Vector3f old_translation;
            Vector3f new_translation;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);

    /// Compensates for a change in translation caused by clipping.
    void AdjustTranslation_(const Vector3f &old_mesh_offset,
                            ExecData_::PerModel &pm);
};
