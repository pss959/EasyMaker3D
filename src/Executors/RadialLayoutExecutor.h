#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

class RadialLayoutCommand;

/// RadialLayoutExecutor executes the RadialLayoutCommand.
///
/// \ingroup Executors
class RadialLayoutExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "RadialLayoutCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a RadialLayoutCommand.
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

    /// Computes the per-Model rotations and translations for the given
    /// command, stores them in the ExecData_, and applies them to the Models.
    void LayOutModels_(ExecData_ &data, const RadialLayoutCommand &rlc);

    /// Computes and returns positions for the given number of Models.
    std::vector<Point3f> GetModelPositions_(size_t count,
                                            const RadialLayoutCommand &rlc);

    /// Moves a Model to the given position and normal.
    void MoveModel_(ExecData_::PerModel &pm,
                    const Point3f &position, const Vector3f &normal);
};
