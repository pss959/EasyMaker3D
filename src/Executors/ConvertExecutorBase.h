#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Models/ConvertedModel.h"
#include "Selection/SelPath.h"

/// ConvertExecutorBase is an abstract base class for Executor classes that
/// convert selected Model instances to some derived ConvertedModel class.
///
/// \ingroup Executors
class ConvertExecutorBase : public Executor {
  public:
    virtual void Execute(Command &command, Command::Op operation) override;

  protected:
    /// Derived classes must implement this to create a derived ConvertedModel
    /// of the correct type.
    virtual ConvertedModelPtr CreateConvertedModel() = 0;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a Command creating a ConvertedModel.
    struct ExecData_ : public Command::ExecData {
        struct PerModel {
            SelPath           path_to_model;    ///< Path to Model to convert.
            ModelPtr          original_model;   ///< Model to convert.
            ConvertedModelPtr converted_model;  ///< Resulting ConvertedModel.
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};

