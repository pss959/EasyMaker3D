//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

DECL_SHARED_PTR(Model);

class PasteCommand;

/// PasteExecutor executes the PasteCommand.
///
/// \ingroup Executors
class PasteExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "PasteCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a PasteCommand.
    struct ExecData_ : public Command::ExecData {
        std::vector<ModelPtr> models;
        SelPath               path_to_parent;  ///< Top-level if empty.
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);

    /// Assigns a unique clone name to the given Model and sets it up for click
    /// interaction. Recurses on any descendant Models.
    void SetUpPastedModel_(Model &model);

    /// Recursively removes all names created for a pasted model.
    void RemoveNames_(Model &model);
};
