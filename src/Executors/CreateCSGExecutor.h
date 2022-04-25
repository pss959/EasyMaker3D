#pragma once

#include "Executors/CombineExecutorBase.h"

/// CreateCSGExecutor executes the CreateCSGModelCommand.
///
/// \ingroup Executors
class CreateCSGExecutor : public CombineExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CreateCSGModelCommand";
    }

  protected:
    virtual CombinedModelPtr CreateCombinedModel(Command &command) override;
};
