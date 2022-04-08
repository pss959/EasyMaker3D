﻿#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Profile.h"
#include "Math/Types.h"
#include "SelPath.h"

/// ChangeRevSurfExecutor executes the ChangeRevSurfCommand.
///
/// \ingroup Executors
class ChangeRevSurfExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeRevSurfCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeRevSurfCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath path_to_model;
            Profile old_profile;
            Anglef  old_sweep_angle;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
