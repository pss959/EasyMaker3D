#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Spin.h"
#include "Math/Types.h"
#include "Selection/SelPath.h"

class Model;

/// ChangeSpinExecutor is an abstract base class for executors that execute a
/// derived ChangeSpinCommand.
///
/// \ingroup Executors
class ChangeSpinExecutor : public Executor {
  public:
    virtual void Execute(Command &command, Command::Op operation) override;

  protected:
    /// Derived classes must implement this to return the current Spin from the
    /// given Model.
    virtual Spin GetModelSpin(const Model &model) const = 0;

    /// Derived classes must implement this to change the current Spin in the
    /// given Model.
    virtual void SetModelSpin(Model &model, const Spin &spin) const = 0;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeSpinCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            Spin     old_spin;
            Vector3f old_translation;
            Vector3f new_translation;
            Vector3f base_translation;  /// Without any offset.
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
