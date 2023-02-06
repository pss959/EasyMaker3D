#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

/// LinearLayoutExecutor executes the LinearLayoutCommand.
///
/// \ingroup Executors
class LinearLayoutExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "LinearLayoutCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a LinearLayoutCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            Vector3f old_translation;
            Vector3f new_translation;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);

    /// Computes the per-Model translations for the given command offset,
    /// stores them in the ExecData_, and applies them to the Models.
    void LayOutModels_(ExecData_ &data, const Vector3f &offset);
};
