//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Profile.h"
#include "Math/Types.h"
#include "Selection/SelPath.h"

/// ChangeRevSurfExecutor executes the ChangeRevSurfCommand.
///
/// \ingroup Executors
class ChangeRevSurfExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeRevSurfCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeRevSurfCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            Profile  old_profile;
            Anglef   old_sweep_angle;
            Vector3f old_translation;
            Vector3f new_translation;
            Vector3f base_translation;  ///< Translation without any offset.
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
