#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Types.h"
#include "SelPath.h"

class RotateCommand;

/// RotateExecutor executes the RotateCommand.
///
/// \ingroup Executors
class RotateExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "RotateCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command.ExecData class that stores everything needed to undo
    /// and redo a RotateCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath   path_to_model;
            Rotationf old_rotation;
            Rotationf new_rotation;
            Vector3f  old_translation;
            Vector3f  new_translation;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);

    /// Computes the per-Model rotations for an in-place rotation for the given
    /// command, stores them in the ExecData_, and applies them to the Models.
    void RotateInPlace_(const RotateCommand &sc, ExecData_ &data);

    /// Computes the per-Model rotations for a regular rotation for the given
    /// command, stores them in the ExecData_, and applies them to the Models.
    void RotateAroundPrimary_(const RotateCommand &sc, ExecData_ &data);

    /// Returns the command's rotation in stage coordinates using the given
    /// matrix to transform it unless rotation is axis-aligned.
    Rotationf GetStageRotation_(const RotateCommand &rc, const Matrix4f &mat);

    /// Composes two rotations in the proper order. Note that when axis-aligned
    /// rotation is in effect the order is reversed.
    static Rotationf ComposeRotations_(const Rotationf &r0, const Rotationf &r1,
                                       bool is_axis_aligned);
};
