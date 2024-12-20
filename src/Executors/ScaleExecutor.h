//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

class ScaleCommand;

/// ScaleExecutor executes the ScaleCommand.
///
/// \ingroup Executors
class ScaleExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ScaleCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ScaleCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            Vector3f old_scale;
            Vector3f new_scale;
            Vector3f old_translation;
            Vector3f new_translation;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);

    /// Computes the per-Model scales (and possibly translations) for the given
    /// command, stores them in the ExecData_, and applies them to the Models.
    void ScaleModels_(const ScaleCommand &sc, ExecData_ &data);

    /// Compensates for a change in translation caused by asymmetric scaling.
    void AdjustTranslation_(const Vector3f &ratios, ExecData_::PerModel &pm);

    /// Compensates for a change in translation caused by symmetric scaling
    /// about the Model base center..
    void AdjustBaseTranslation_(ExecData_::PerModel &pm);
};
