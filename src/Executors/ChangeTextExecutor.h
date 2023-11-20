#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Selection/SelPath.h"

/// ChangeTextExecutor executes the ChangeTextCommand.
///
/// \ingroup Executors
class ChangeTextExecutor : public Executor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeTextCommand";
    }

    virtual void Execute(Command &command, Command::Op operation) override;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a ChangeTextCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath path_to_model;
            Str     old_text_string;
            Str     old_font_name;
            float   old_char_spacing;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
