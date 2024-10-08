//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Models/CombinedModel.h"
#include "Selection/SelPath.h"

/// CombineExecutorBase is an abstract base class for Executor classes dealing
/// with CombinedModel creation.
///
/// \ingroup Executors
class CombineExecutorBase : public Executor {
  public:
    virtual void Execute(Command &command, Command::Op operation) override;

  protected:
    /// Derived classes are required to implement this to actually create a
    /// CombinedModel. If the given name is not empty, it should be used for
    /// the new CombinedModel.
    virtual CombinedModelPtr CreateCombinedModel(
        Command &command, const Str &name) = 0;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a Command creating a CombinedModel.
    struct ExecData_ : public Command::ExecData {
        std::vector<SelPath>  paths_to_models;  ///< Paths to Models to combine.
        CombinedModelPtr      combined_model;   ///< Model that was created.
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};

