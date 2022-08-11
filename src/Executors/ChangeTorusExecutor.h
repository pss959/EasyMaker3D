#pragma once

#include <vector>

#include "App/SelPath.h"
#include "Executors/Executor.h"

/// ChangeTorusExecutor executes the ChangeTorusCommand.
///
/// \ingroup Executors
class ChangeTorusExecutor : public Executor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeTorusCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeTorusCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            float    old_radius;
            bool     do_translate;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);

    /// Returns true if the torus in the given PerModel data is resting on the
    /// stage and should be translated up or down if the inner radius changes.
    bool IsOnStage_(const ExecData_::PerModel &pm) const;
};
