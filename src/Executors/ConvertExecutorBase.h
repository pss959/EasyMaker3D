#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Models/ConvertedModel.h"
#include "SelPath.h"

/// ConvertExecutorBase is an abstract base class for Executor classes that
/// convert selected Model instances to some derived ConvertedModel class.
///
/// \ingroup Executors
class ConvertExecutorBase : public Executor {
  public:
    virtual void Execute(Command &command, Command::Op operation) override;

  protected:
    /// Derived classes are required to implement this to actually convert a
    /// model to a derived ConvertedModel class. Note that if the Model is
    /// already of the correct type, it should be left alone. The
    /// ConvertedModel for the primary selection is passed in to help with the
    /// conversion (e.g., for copying data); this will be null for the primary
    /// selection itself.
    virtual ConvertedModelPtr ConvertModel(
        const ModelPtr &model, const ConvertedModelPtr &primary) = 0;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a Command creating a ConvertedModel.
    struct ExecData_ : public Command::ExecData {
        struct PerModel {
            SelPath           path_to_model;    ///< Path to Model to convert.
            ConvertedModelPtr converted_model;  ///< Resulting ConvertedModel.
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};

